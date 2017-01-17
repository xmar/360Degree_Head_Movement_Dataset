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
}

#include "Buffer.hpp"
#include "DisplayFrameInfo.hpp"

namespace IMT {
namespace LibAv {

class Frame;

class VideoReader
{
    public:
        VideoReader(std::string inputPath, size_t bufferSize = 10);
        VideoReader(const VideoReader&) = delete;
        VideoReader& operator=(const VideoReader&) = delete;

        virtual ~VideoReader(void);

        void Init(unsigned nbFrames);

        //#ifdef USE_OPENGL
        //Update the current binded OpenGL Texture object with the content of the next picture (if right deadline)
        //return the current frame info
        IMT::DisplayFrameInfo SetNextPictureToOpenGLTexture(unsigned streamId, std::chrono::system_clock::time_point deadline);
        //#endif

        unsigned GetNbStream(void) const {return m_videoStreamIds.size();}

    protected:

    private:
        std::string m_inputPath;
        AVFormatContext* m_fmt_ctx;
        std::vector<unsigned int> m_videoStreamIds;
        std::map<unsigned int, unsigned int> m_streamIdToVecId;
        //First version: we totaly decode the video and store in a vector the output frames
        //std::vector<std::queue<std::shared_ptr<cv::Mat>>> m_outputFrames;
        //std::vector<std::queue<std::shared_ptr<Frame>>> m_outputFrames;
        IMT::Buffer<Frame> m_outputFrames;
        unsigned m_nbFrames;
        std::vector<bool> m_doneVect;
        std::vector<bool> m_gotOne;
        std::chrono::system_clock::time_point m_startDisplayTime;
        struct SwsContext* m_swsCtx;
        AVFrame* m_frame_ptr2;
        std::thread m_decodingThread;
        size_t m_lastDisplayedPictureNumber;

        void RunDecoderThread(void);
};
}
}
