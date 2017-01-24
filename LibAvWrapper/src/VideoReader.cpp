#include "VideoReader.hpp"
#include "Frame.hpp"

//#ifdef USE_OPENGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
//#endif

extern "C"
{
  #include "libavutil/opt.h"
}

#include <iostream>
#include <Packet.hpp>
#include <stdexcept>

#define DEBUG_VideoReader 0
#if DEBUG_VideoReader
#define PRINT_DEBUG_VideoReader(s) std::cout << "DEC -- " << s << std::endl;
#else
#define PRINT_DEBUG_VideoReader(s) {}
#endif // DEBUG_VideoReader

extern "C"
{
   #include <libavutil/imgutils.h>
//   #include <libavutil/parseutils.h>
   #include <libswscale/swscale.h>
}

constexpr size_t SDL_AUDIO_BUFFER_SIZE = 1024;

using namespace IMT::LibAv;

VideoReader::VideoReader(std::string inputPath, size_t bufferSize): m_inputPath(inputPath), m_fmt_ctx(nullptr), m_videoStreamIds(),
    m_outputFrames(bufferSize), m_outputAudioFrames(-1), m_streamIdToVecId(), m_nbFrames(0), m_doneVect(), m_gotOne(), m_startDisplayTime(std::chrono::system_clock::now()),
    m_swsCtx(nullptr), m_audioSwrCtx(nullptr), m_frame_ptr2(nullptr), m_decodingThread(), m_lastDisplayedPictureNumber(-1),
    m_videoStreamId(-1), m_audioStreamId(-1), m_lastPlayedAudioFrame(nullptr)
{
}

VideoReader::~VideoReader()
{
  SDL_PauseAudio(1);
  if (m_decodingThread.joinable())
  {
    std::cout << "Join decoding thread\n";
    m_outputFrames.Stop();
    m_outputAudioFrames.Stop();
    m_decodingThread.join();
    std::cout << "Join decoding thread: done\n";
  }
  if (m_fmt_ctx != nullptr)
  {
    for (unsigned i = 0; i < m_fmt_ctx->nb_streams; ++i)
    {
      avcodec_close(m_fmt_ctx->streams[i]->codec);
      //av_free(m_fmt_ctx->streams[i]->codec);
    }
    avformat_close_input(&m_fmt_ctx);
    avformat_free_context(m_fmt_ctx);
    m_fmt_ctx = nullptr;
  }
  if (m_frame_ptr2 != nullptr)
  {
    av_freep(&m_frame_ptr2->data[0]);
    av_frame_unref(m_frame_ptr2);
    av_frame_free(&m_frame_ptr2);
  }
  if (m_audioSwrCtx != nullptr)
  {

  }
}

void printA(AVFormatContext* _a)
{
std::cout << "duration    " << (unsigned long)_a->duration << "\n";
std::cout << "streams     " << _a->nb_streams << "\n";
unsigned nbVideo = 0;
for (unsigned i = 0; i < _a->nb_streams; ++i)
{
    if(_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        nbVideo++;
    }
}
std::cout << "vid stream  " << nbVideo << "\n";
std::cout << "format name " << _a->iformat->name << "\n";
std::cout << "bit_rate    " << _a->bit_rate << "\n";
std::cout << "long name   " << _a->iformat->long_name << "\n";
}

void VideoReader::Init(unsigned nbFrames)
{
    m_nbFrames = nbFrames;
    PRINT_DEBUG_VideoReader("Register codecs");
    av_register_all();

    int ret = 0;

//    uint8_t* buffer = nullptr;
//    size_t buffer_size;
//    int ret = av_file_map(m_inputPath.c_str(), &buffer, &buffer_size, 0, nullptr);
//    if (ret < 0)
//    {
//        std::cout << "Error while slurping the file content into the buffer" << std::endl;
//    }

//    struct buffer_data bd = { 0 };
//    bd.ptr  = buffer;
//    bd.size = buffer_size;

    PRINT_DEBUG_VideoReader("Allocate format context");
    if (!(m_fmt_ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        std::cout << "Error while allocating the format context" << std::endl;
    }

    PRINT_DEBUG_VideoReader("Open format context: "<<m_inputPath);
    ret = avformat_open_input(&m_fmt_ctx, m_inputPath.c_str(), nullptr, nullptr);
    if (ret < 0) {
        std::cout << "Could not open input" << std::endl;
    }

    PRINT_DEBUG_VideoReader("Find streams info");
    ret = avformat_find_stream_info(m_fmt_ctx, nullptr);
    if (ret < 0) {
        std::cout << "Could not find stream information" << std::endl;
    }

    PRINT_DEBUG_VideoReader("Dump format context");
    av_dump_format(m_fmt_ctx, 0, m_inputPath.c_str(), 0);

    printA(m_fmt_ctx);

    PRINT_DEBUG_VideoReader("Init video stream decoders");
    if (m_fmt_ctx->nb_streams > 2)
    {
      throw(std::invalid_argument("Support only video with one video stream and one audio stream"));
    }
    for (unsigned i = 0; i < m_fmt_ctx->nb_streams; ++i)
    {
        if(m_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoStreamId = i;
            m_fmt_ctx->streams[i]->codec->refcounted_frames = 1;
            //m_outputFrames.emplace_back();
            m_streamIdToVecId[i] = m_videoStreamIds.size();
            m_videoStreamIds.push_back(i);
            auto* decoder = avcodec_find_decoder(m_fmt_ctx->streams[i]->codec->codec_id);
            if(!decoder)
            {
                std::cout << "Could not find the decoder for stream id " << i << std::endl;
            }
            PRINT_DEBUG_VideoReader("Init decoder for stream id " << i);
            if ((ret = avcodec_open2(m_fmt_ctx->streams[i]->codec, decoder, nullptr)) < 0)
            {
                std::cout << "Could not open the decoder for stream id " << i << std::endl;
            }
        }
        else if (m_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
          m_audioStreamId = i;
          m_fmt_ctx->streams[i]->codec->refcounted_frames = 1;
          m_fmt_ctx->streams[i]->codec->request_sample_fmt = AV_SAMPLE_FMT_S16;

          auto* decoder = avcodec_find_decoder(m_fmt_ctx->streams[i]->codec->codec_id);
          if(!decoder)
          {
              std::cout << "Could not find the audio decoder for stream id " << i << std::endl;
          }
          PRINT_DEBUG_VideoReader("Init audio decoder for stream id " << i);
          if ((ret = avcodec_open2(m_fmt_ctx->streams[i]->codec, decoder, nullptr)) < 0)
          {
              std::cout << "Could not open the decoder for stream id " << i << std::endl;
          }
          std::cout << "Got an Audio track" << std::endl;


          // Some MP3/WAV don't tell this so make assumtion that
          // They are stereo not 5.1
          if (m_fmt_ctx->streams[i]->codec->channel_layout == 0
                  && m_fmt_ctx->streams[i]->codec->channels == 2)
          {
              m_fmt_ctx->streams[i]->codec->channel_layout = AV_CH_LAYOUT_STEREO;
          }
          else if (m_fmt_ctx->streams[i]->codec->channel_layout == 0
                     && m_fmt_ctx->streams[i]->codec->channels == 1)
          {
              m_fmt_ctx->streams[i]->codec->channel_layout = AV_CH_LAYOUT_MONO;
          }
          else if (m_fmt_ctx->streams[i]->codec->channel_layout == 0
                     && m_fmt_ctx->streams[i]->codec->channels == 0)
          {
              m_fmt_ctx->streams[i]->codec->channel_layout = AV_CH_LAYOUT_STEREO;
              m_fmt_ctx->streams[i]->codec->channels = 2;
          }

          //init resampler
          if (m_fmt_ctx->streams[i]->codec->sample_fmt != AV_SAMPLE_FMT_S16)
          {
            m_audioSwrCtx = avresample_alloc_context();
            av_opt_set_int(m_audioSwrCtx, "in_channel_layout",
                           m_fmt_ctx->streams[i]->codec->channel_layout, 0);
           av_opt_set_int(m_audioSwrCtx, "in_channels",
                          m_fmt_ctx->streams[i]->codec->channels, 0);
            av_opt_set_int(m_audioSwrCtx, "in_sample_fmt",
                           m_fmt_ctx->streams[i]->codec->sample_fmt, 0);
            av_opt_set_int(m_audioSwrCtx, "in_sample_rate",
                           m_fmt_ctx->streams[i]->codec->sample_rate, 0);

            av_opt_set_int(m_audioSwrCtx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
            av_opt_set_int(m_audioSwrCtx, "out_channels", 2, 0);
            av_opt_set_int(m_audioSwrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
            av_opt_set_int(m_audioSwrCtx, "out_sample_rate", m_fmt_ctx->streams[i]->codec->sample_rate, 0);
          }

          if (avresample_open(m_audioSwrCtx) < 0)
          {
            std::cerr << "Cannot open avresample" << std::endl;
            m_audioSwrCtx = nullptr;
          }
        }
    }
    m_outputFrames.SetTotal(nbFrames);
    m_outputAudioFrames.SetTotal(5*nbFrames);
    std::cout << "Nb frames = " << nbFrames << std::endl;
    m_doneVect = std::vector<bool>(1, false);
    m_gotOne = std::vector<bool>(1, false);

    std::cout << "Start decoding thread\n";
    m_decodingThread = std::thread(&VideoReader::RunDecoderThread, this);
}

void VideoReader::Audio_callback(void* userdata, unsigned char* stream, int len)
{
    auto* videoReader = static_cast<VideoReader*>(userdata);

    SDL_memset(stream, 0, len);  // make sure this is silence.

    auto now = std::chrono::system_clock::now();
    auto deadline = std::chrono::system_clock::time_point(now - videoReader->m_startDisplayTime);

    while(len > 0)
    {
      std::shared_ptr<AudioFrame> audioFrame = videoReader->m_lastPlayedAudioFrame;
      bool done = audioFrame != nullptr;
      if (done)
      {//Test if the lastPlayedAudioFrame is still valid
        auto pts = audioFrame->GetDisplayTimestamp();
        if (pts > deadline)
        { // not valid anymore, we will try to get a new one
          done = false;
          audioFrame = nullptr;
          videoReader->m_lastPlayedAudioFrame = nullptr;
        }
      }
      while(!done)
      {
        auto tmp_audioFrame = videoReader->m_outputAudioFrames.Get();
        if (tmp_audioFrame != nullptr)
        {
          auto pts = tmp_audioFrame->GetDisplayTimestamp();
          if (pts < deadline)
          {
            audioFrame = tmp_audioFrame;
            videoReader->m_outputAudioFrames.Pop();
          }
          else
          {
            done = true;
          }
        }
        else
        {
          done = true;
        }
      }
      if (audioFrame != nullptr && audioFrame->IsValid())
      {
        //std::cout << "Audio frame \n";
        auto pts = audioFrame->GetDisplayTimestamp();
        //std::cout << pts.time_since_epoch().count() << " < " << deadline.time_since_epoch().count() << "\n";
        if (audioFrame->SetOutputAudioBuff(stream, len))
        {
          videoReader->m_lastPlayedAudioFrame = audioFrame;
        }
        else
        {
          videoReader->m_lastPlayedAudioFrame = nullptr;
        }
      }
      else
      {
        memset(stream, 0x00, len);
        len = 0;
      }
    }


    // long len1, audio_size;
    // double pts;
    //
    // while(len > 0) {
    //     if(is->audio_buf_index >= is->audio_buf_size) {
    //         /* We have already sent all our data; get more */
    //         audio_size = audio_decode_frame(is, &pts);
    //
    //         if(audio_size < 0) {
    //             /* If error, output silence */
    //             is->audio_buf_size = 1024;
    //             memset(is->audio_buf, 0, is->audio_buf_size);
    //
    //         } else {
    //             is->audio_buf_size = audio_size;
    //         }
    //
    //         is->audio_buf_index = 0;
    //     }
    //
    //     len1 = is->audio_buf_size - is->audio_buf_index;
    //
    //     if(len1 > len) {
    //         len1 = len;
    //     }
    //
    //     memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
    //     len -= len1;
    //     stream += len1;
    //     is->audio_buf_index += len1;
    // }
}

void VideoReader::InitAudio(void)
{
  if (m_audioStreamId != size_t(-1))
  {
    std::cout << "Audio stream detected: init SDL audio" << std::endl;
    SDL_AudioSpec wanted_spec, spec;
    SDL_zero(wanted_spec);
    wanted_spec.freq = m_fmt_ctx->streams[m_audioStreamId]->codec->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = 2;//m_fmt_ctx->streams[m_audioStreamId]->codec->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    wanted_spec.callback = &VideoReader::Audio_callback;
    wanted_spec.userdata = this;

    if (SDL_OpenAudio(&wanted_spec, &spec) < 0)
    {
      std::cerr << "SDL_OpenAudio: " <<  SDL_GetError() << "\n";
      throw(std::invalid_argument("Impossible to init Audio with the SDL"));
    }
  }
}

void VideoReader::RunDecoderThread(void)
{
    AVPacket pkt;
    int ret = -1;
    PRINT_DEBUG_VideoReader("Read next pkt")
    while ((ret = av_read_frame(m_fmt_ctx, &pkt)) >= 0)
    {
        unsigned streamId = pkt.stream_index;
        if (streamId == m_videoStreamId)
        {
          if (m_streamIdToVecId.count(streamId) > 0 && !m_doneVect[m_streamIdToVecId[streamId]])
          { //then the pkt belong to a stream we care about.
              PRINT_DEBUG_VideoReader("Got a pkt for streamId "<<streamId)
              const AVPacket* const packet_ptr = &pkt;
              bool got_a_frame = false;
              auto* codecCtx = m_fmt_ctx->streams[streamId]->codec;
              PRINT_DEBUG_VideoReader("Send the packet to decoder for streamId "<<streamId)
              ret = avcodec_send_packet(codecCtx, packet_ptr);
              if (ret == 0)
              {//success to send packet
                  while(true)
                  {
                    PRINT_DEBUG_VideoReader("Ask if a frame is available for streamId " <<streamId)
                    auto frame = std::make_shared<VideoFrame>();
                    ret = frame->AvCodecReceiveFrame(codecCtx);
                    got_a_frame = got_a_frame || (ret == 0);
                    if (ret == 0)
                    {
                        frame->SetTimeBase(m_fmt_ctx->streams[streamId]->time_base);
                        PRINT_DEBUG_VideoReader("Got a frame for streamId " <<streamId)
                        m_gotOne[m_streamIdToVecId[streamId]] = true;
                        //m_outputFrames[m_streamIdToVecId[streamId]].push(std::move(frame));
                        if (!m_outputFrames.Add(std::move(frame)))
                        {
                          std::cout << "Decoding thread stopped: frame limite exceed\n";
                          return;
                        }
                    }
                    else
                    {
                      PRINT_DEBUG_VideoReader("No frame available for streamId " <<streamId)
                      break;
                    }
                  }
              }
              //m_doneVect[m_streamIdToVecId[streamId]] = (m_gotOne[m_streamIdToVecId[streamId]] && (!got_a_frame)) || (m_outputFrames[m_streamIdToVecId[streamId]].size() >= m_nbFrames);
          }
        }
        else if (streamId == m_audioStreamId)
        {
          PRINT_DEBUG_VideoReader("Got an audio pkt for streamId "<<streamId)
          const AVPacket* const packet_ptr = &pkt;
          bool got_a_frame = false;
          auto* codecCtx = m_fmt_ctx->streams[streamId]->codec;
          PRINT_DEBUG_VideoReader("Send the audio packet to decoder for streamId "<<streamId)
          auto audioFrame = std::make_shared<AudioFrame>();
          auto ret = audioFrame->AvCodecReceiveAudioFrame(codecCtx, m_audioSwrCtx, packet_ptr);
          if (ret > 0)
          {
            audioFrame->SetTimeBase(m_fmt_ctx->streams[streamId]->time_base);
            m_outputAudioFrames.Add(audioFrame);
          }
        }
        av_packet_unref(&pkt);
    }
    { //flush all the rest
        PRINT_DEBUG_VideoReader("Start to flush")
        if (m_videoStreamId != size_t(-1))
        {
          //send flush signal
          PRINT_DEBUG_VideoReader("Send flush signal for streamId "<<m_videoStreamId)
          avcodec_send_packet(m_fmt_ctx->streams[m_videoStreamIds[m_videoStreamId]]->codec, nullptr);
          while(true)//we decode all the reste of the frames
          {
              if (!m_doneVect[m_videoStreamId])
              {

                  bool got_a_frame = false;
                  auto* codecCtx = m_fmt_ctx->streams[m_videoStreamIds[m_videoStreamId]]->codec;
                  PRINT_DEBUG_VideoReader("Ask for next frame for streamVectId "<<m_videoStreamId)
                  auto frame = std::make_shared<VideoFrame>();
                  int ret = frame->AvCodecReceiveFrame(m_fmt_ctx->streams[m_videoStreamIds[m_videoStreamId]]->codec);
                  got_a_frame = ret == 0;
                  if (got_a_frame)
                  {
                      PRINT_DEBUG_VideoReader("Got a frame for streamVectId "<<m_videoStreamId)
                      if(!m_outputFrames.Add(std::move(frame)))
                      {
                        std::cout << "Decoding thread stopped: frame limite exceed\n";
                        return;
                      }
                      //m_outputFrames[streamVectId].emplace();
                  }
                  else
                  {
                    std::cout << "Decoding thread stopped: video done\n";
                    m_outputFrames.SetTotal(0);
                    return;
                  }
                  //m_doneVect[streamVectId] = (!got_a_frame) || (m_outputFrames[streamVectId].size() >= m_nbFrames);
                  //streamVectId = (streamVectId + 1) % m_outputFrames.size();
              }
          }
        }
        if (m_audioStreamId != size_t(-1))
        {
          //send flush signal
          PRINT_DEBUG_VideoReader("Send flush signal for streamId "<<m_audioStreamId)
          avcodec_send_packet(m_fmt_ctx->streams[m_audioStreamId]->codec, nullptr);
          while(true)//we decode all the reste of the frames
          {
              bool got_a_frame = false;
              auto* codecCtx = m_fmt_ctx->streams[m_audioStreamId]->codec;
              PRINT_DEBUG_VideoReader("Ask for next frame for streamVectId "<<m_audioStreamId)
              auto audioFrame = std::make_shared<AudioFrame>();
              int ret = audioFrame->AvCodecReceiveAudioFrame(codecCtx, m_audioSwrCtx, nullptr);
              got_a_frame = ret == 0;
              if (got_a_frame)
              {
                audioFrame->SetTimeBase(m_fmt_ctx->streams[m_audioStreamId]->time_base);
                m_outputAudioFrames.Add(audioFrame);
              }
              else
              {
                std::cout << "Decoding thread stopped: audio done\n";
                m_outputAudioFrames.SetTotal(0);
                return;
              }
          }
        }
    }
}

IMT::DisplayFrameInfo VideoReader::SetNextPictureToOpenGLTexture(std::chrono::system_clock::time_point deadline)
{
    bool first = (m_swsCtx == nullptr);
    bool last = false;
    auto pts = std::chrono::system_clock::time_point(std::chrono::seconds(-1));
    size_t nbUsed = 0;
    if (0 < 1)
    {
        if (!m_outputFrames.IsAllDones())
        {
            PRINT_DEBUG_VideoReader("Update video picture")
            std::shared_ptr<VideoFrame> frame(nullptr);
            bool done = false;
            while(!done)
            {
              auto tmp_frame = m_outputFrames.Get();
              if (tmp_frame != nullptr)
              {
                if (deadline >= tmp_frame->GetDisplayTimestamp())
                {
                  pts = tmp_frame->GetDisplayTimestamp();
                  frame = std::move(tmp_frame);
                  m_outputFrames.Pop();
                  ++m_lastDisplayedPictureNumber;
                  ++nbUsed;
                }
                else
                {
                  done = true;
                }
              }
              else
              {
                done = true;
              }
            }

            if (frame != nullptr && frame->IsValid())
            {
              auto w = frame->GetWidth();
              auto h = frame->GetHeight();
              if (first)
              {
                if (m_audioStreamId != size_t(-1))
                {
                  std::cout << "Start the audio\n";
                  SDL_PauseAudio(0);
                }


                m_swsCtx = sws_getContext(w, h,
                                  AV_PIX_FMT_YUV420P, w, h,
                                  AV_PIX_FMT_RGB24, 0, 0, 0, 0);
                m_frame_ptr2 = av_frame_alloc();
                av_image_alloc(m_frame_ptr2->data, m_frame_ptr2->linesize, w, h, AV_PIX_FMT_BGR24, 1);
              }
              //transform from YUV420p domain to RGB domain
              sws_scale(m_swsCtx, frame->GetDataPtr(), frame->GetRowLength(), 0, h, m_frame_ptr2->data, m_frame_ptr2->linesize);

              if (first)
              {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_frame_ptr2->data[0]);
              }
              else
              {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, m_frame_ptr2->data[0]);
              }


              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
              // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
              glGenerateMipmap(GL_TEXTURE_2D);
            }
            else if (frame != nullptr && !frame->IsValid())
            {
              last = true;
              //Stop sound
              SDL_PauseAudio(1);
            }
        }
        else
        {
          last = true;
          //Stop sound
          SDL_PauseAudio(1);
        }
    }
    return {m_lastDisplayedPictureNumber, nbUsed > 0 ? nbUsed - 1 : 0, deadline, pts, last};
}
