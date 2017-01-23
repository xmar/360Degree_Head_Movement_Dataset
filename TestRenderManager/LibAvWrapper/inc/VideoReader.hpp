#pragma once
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <chrono>
#include <thread>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavutil/file.h>
    #include <libswscale/swscale.h>
    #include <libavresample/avresample.h>
}

#include "Buffer.hpp"
#include "DisplayFrameInfo.hpp"

namespace IMT {
namespace LibAv {

class VideoFrame;
class AudioFrame;

class VideoReader
{
    public:
        VideoReader(std::string inputPath, size_t bufferSize = 10);
        VideoReader(const VideoReader&) = delete;
        VideoReader& operator=(const VideoReader&) = delete;

        virtual ~VideoReader(void);

        void Init(unsigned nbFrames);
        void InitAudio(void);

        //#ifdef USE_OPENGL
        //Update the current binded OpenGL Texture object with the content of the next picture (if right deadline)
        //return the current frame info
        IMT::DisplayFrameInfo SetNextPictureToOpenGLTexture(std::chrono::system_clock::time_point deadline);
        //#endif

        unsigned GetNbStream(void) const {return m_videoStreamIds.size();}

        void SetStartTime(std::chrono::system_clock::time_point startTime) {m_startDisplayTime = std::move(startTime);}

    protected:

    private:
        std::string m_inputPath;
        AVFormatContext* m_fmt_ctx;
        std::vector<unsigned int> m_videoStreamIds;
        std::map<unsigned int, unsigned int> m_streamIdToVecId;
        IMT::Buffer<VideoFrame> m_outputFrames;
        IMT::Buffer<AudioFrame> m_outputAudioFrames;
        unsigned m_nbFrames;
        std::vector<bool> m_doneVect;
        std::vector<bool> m_gotOne;
        std::chrono::system_clock::time_point m_startDisplayTime; //this time will be used to sync the video and the audio. The master clock is the system_clock
        struct SwsContext* m_swsCtx;
        AVAudioResampleContext* m_audioSwrCtx;
        AVFrame* m_frame_ptr2;
        std::thread m_decodingThread;
        size_t m_lastDisplayedPictureNumber;
        size_t m_videoStreamId;
        size_t m_audioStreamId;
        std::shared_ptr<AudioFrame> m_lastPlayedAudioFrame;

        void RunDecoderThread(void);
        static void Audio_callback(void* userdata, unsigned char* stream, int len);
};
}
}
