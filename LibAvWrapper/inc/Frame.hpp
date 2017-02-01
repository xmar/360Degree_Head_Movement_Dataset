#pragma once

extern "C"
{
   #include <libavutil/imgutils.h>
//   #include <libavutil/parseutils.h>
   #include <libswscale/swscale.h>

   #include <libavresample/avresample.h>
   #include "libavutil/opt.h"
}
#include <chrono>

#include <iostream>

namespace IMT {
namespace LibAv {

class Frame
{
public:
  Frame(void): m_framePtr(nullptr), m_time_base{0,0}, m_haveFrame(false),
  m_timeOffset(0)
  {
    m_framePtr = av_frame_alloc();
  }
  virtual ~Frame(void)
  {
    if (IsValid())
    {
      av_frame_unref(m_framePtr);
      m_haveFrame = -1;
    }
    av_frame_free(&m_framePtr);
    m_framePtr = nullptr;
  }

  bool IsValid(void) const {return m_haveFrame;}
  void SetTimeBase(AVRational time_base) {m_time_base = std::move(time_base);}
  void SetTimeOffset(const std::chrono::milliseconds& timeOffset) {m_timeOffset = timeOffset;}
  auto GetDisplayTimestamp(void) const {if (IsValid()) {return std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>((double(m_time_base.num)*av_frame_get_best_effort_timestamp(m_framePtr))/m_time_base.den))) - m_timeOffset;} else {return std::chrono::system_clock::time_point(std::chrono::milliseconds(-1));}}
  uint8_t** GetDataPtr(void) {if (IsValid()) {return m_framePtr->data;} else {return nullptr;}}
  size_t GetDisplayPictureNumber(void) const {if (IsValid()) {return m_framePtr->display_picture_number;} else {return -1;}}
protected:
  bool m_haveFrame;
  AVFrame* m_framePtr;
private:
  Frame(Frame const&) = delete;
  Frame& operator=(Frame const&) = delete;

  AVRational m_time_base;
  std::chrono::milliseconds m_timeOffset;
};

class VideoFrame final: public Frame
{
public:
  VideoFrame(void): Frame() {}
  virtual ~VideoFrame(void) = default;

  auto AvCodecReceiveFrame(AVCodecContext* codecCtx)  {auto ret = avcodec_receive_frame(codecCtx, m_framePtr); m_haveFrame = (ret == 0); return ret;}
  int* GetRowLength(void) {if (IsValid()) {return m_framePtr->linesize;} else {return nullptr;}}
  int GetWidth(void) const {if (IsValid()) {return m_framePtr->width;} else {return -1;}}
  int GetHeight(void) const {if (IsValid()) {return m_framePtr->height;} else {return -1;}}
};

class AudioFrame final: public Frame
{
public:
  AudioFrame(void): Frame(), m_frameSize(-1), m_remainingSize(-1), m_resampledOut(nullptr), m_currentReadingPosition(nullptr) {}
  virtual ~AudioFrame(void)
  {
    if (m_resampledOut != nullptr)
    {
      av_freep(&m_resampledOut);
      m_resampledOut = nullptr;
    }
    m_currentReadingPosition = nullptr;
  }

  auto AvCodecReceiveAudioFrame(AVCodecContext* codecCtx, AVAudioResampleContext* audioSwrCtx, const AVPacket* const packet_ptr)
  {
    avcodec_decode_audio4(codecCtx, m_framePtr, &m_frameSize, packet_ptr);
    m_haveFrame = m_frameSize > 0;
    if (m_haveFrame)
    {
      auto data_size =
        av_samples_get_buffer_size(nullptr, codecCtx->channels, m_framePtr->nb_samples, codecCtx->sample_fmt, 1);
      if (audioSwrCtx != nullptr)
      {
        //Start resampling
        // There is pre 1.0 libavresample and then there is above..
        #if LIBAVRESAMPLE_VERSION_MAJOR == 0
            void** resample_input_bytes = static_cast<void **>(m_framePtr->extended_data);
        #else
            uint8_t** resample_input_bytes = static_cast<uint8_t **>(m_framePtr->extended_data);
        #endif
        int64_t outSampleRate = 0;
        av_opt_get_int(audioSwrCtx, "out_sample_rate", 0, &outSampleRate);
        auto resample_size = av_rescale_rnd(avresample_get_delay(audioSwrCtx) +
                                           m_framePtr->nb_samples,
                                           outSampleRate,
                                           codecCtx->sample_rate,
                                           AV_ROUND_UP);
        int resample_lines;
        constexpr size_t nbChannel = 2;
        av_samples_alloc(&m_resampledOut, &resample_lines, nbChannel, resample_size,
                          AV_SAMPLE_FMT_S16, 0);
        // OLD API (0.0.3) ... still NEW API (1.0.0 and above).. very frustrating..
        // USED IN FFMPEG 1.0 (LibAV SOMETHING!). New in FFMPEG 1.1 and libav 9
        #if LIBAVRESAMPLE_VERSION_INT <= 3
            // AVResample OLD
            auto resample_nblen = avresample_convert(audioSwrCtx, static_cast<void **>(&m_resampledOut), 0,
                                                resample_size,
                                                static_cast<void **>(resample_input_bytes), 0, m_framePtr->nb_samples);
        #else
            //AVResample NEW
            auto resample_nblen = avresample_convert(audioSwrCtx, static_cast<uint8_t **>(&m_resampledOut),
                                                0, resample_size,
                                                static_cast<uint8_t **>(resample_input_bytes), 0, m_framePtr->nb_samples);
        #endif

        int64_t out_sample_fmt;
        av_opt_get_int(audioSwrCtx, "out_sample_fmt", 0, &out_sample_fmt);
        m_remainingSize = av_samples_get_buffer_size(NULL, nbChannel, resample_nblen,
                             static_cast<AVSampleFormat>(out_sample_fmt), 1);
        m_currentReadingPosition = m_resampledOut;
      }
      else
      {
        m_remainingSize = data_size;
        m_currentReadingPosition = static_cast<uint8_t*>(*GetDataPtr());
      }
    }
    return m_frameSize;
  }
  auto GetFrameSize(void) const {return m_frameSize;}
  /** @brief Copy the audio buffer in this frame into the stream buffer. If enough
  data is available, remainingLength is written. Otherwise only what is available is taken.
  remainingLength is updated with the length of the remaining place available in the stream buffer,
  and stream
  Return false if no more data is available in this frame.
  **/
  bool SetOutputAudioBuff(unsigned char*& stream, int& remainingLength)
  {
    int len = std::min(m_remainingSize, remainingLength);
    if (len > 0)
    {
      memcpy(stream, m_currentReadingPosition, len);
      stream += len;
      m_currentReadingPosition += len;
      remainingLength -= len;
      m_remainingSize -= len;
    }
    return m_remainingSize > 0;
  }
private:
  int m_frameSize;
  int m_remainingSize;
  uint8_t* m_resampledOut;
  uint8_t* m_currentReadingPosition;
};

}
}
