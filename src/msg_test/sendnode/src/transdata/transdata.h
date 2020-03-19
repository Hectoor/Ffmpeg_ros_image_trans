#ifndef VERSION1_0_TRANSDATA_H
#define VERSION1_0_TRANSDATA_H

#include <iostream>
#include <thread>
#include <mutex>
extern "C"
{
#include "libavformat/avformat.h"
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
};
#include "opencv2/core.hpp"
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Transdata
{
public:
    Transdata();
    ~Transdata();

    void AVFrame2Img(AVFrame *pFrame, cv::Mat& img);
    void Yuv420p2Rgb32(const uchar *yuvBuffer_in, const uchar *rgbBuffer_out, int width, int height);
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket pkt;
    AVFrame *pframe = NULL;
    int ret, i;
    int videoindex=-1;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    const AVBitStreamFilter *buffersrc  =  NULL;
    AVBSFContext *bsf_ctx;
    AVCodecParameters *codecpar = NULL;
    //const char *in_filename  = "rtmp://localhost:1935/rtmplive";   //芒果台rtmp地址
    //const char *in_filename  = "rtmp://58.200.131.2:1935/livetv/hunantv";   //芒果台rtmp地址
    const char *in_filename = "rtmp://183.62.75.39:6030/livertmp/test";
    const char *out_filename_v = "test1.h264";	//Output file URL

    cv::Mat image_test;
    mutex mImage_buf;
    int Transdata_init();
    int Transdata_Recdata();
    int Transdata_free();
};


#endif //VERSION1_0_TRANSDATA_H
