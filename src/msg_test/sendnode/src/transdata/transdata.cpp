#include "transdata.h"



Transdata::Transdata(){}
Transdata::~Transdata(){}



int Transdata::Transdata_free()
{
    av_bsf_free(&bsf_ctx);
    avformat_close_input(&ifmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf( "Error occurred.\n");
        return -1;
    }
    return 0;
}


int Transdata::Transdata_Recdata()
{
    if(av_read_frame(ifmt_ctx, &pkt)<0) {
        return -1;
    }
    if (pkt.stream_index == videoindex) {
        // H.264 Filter
        if(av_bsf_send_packet(bsf_ctx, &pkt) < 0)
        {
            cout << " bsg_send_packet is error! " << endl;
            return -1;
        }
        if(av_bsf_receive_packet(bsf_ctx, &pkt) < 0)
        {
            cout << " bsg_receive_packet is error! " << endl;
            return -1;
        }
        printf("Write Video Packet. size:%d\tpts:%ld\n", pkt.size, pkt.pts);
        // Decode AVPacket
        if(pkt.size)
        {
            ret = avcodec_send_packet(pCodecCtx, &pkt);
            if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                std::cout << "avcodec_send_packet: " << ret << std::endl;
                return -1;
            }
            //Get AVframe
            ret = avcodec_receive_frame(pCodecCtx, pframe);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                std::cout << "avcodec_receive_frame: " << ret << std::endl;
                return -1;
            }

            //AVframe to rgb  这一步要操作image_test，所以锁住
            mImage_buf.lock();
            AVFrame2Img(pframe,image_test);
            mImage_buf.unlock();
        }
    }
    //Free AvPacket
    av_packet_unref(&pkt);
    return 0;
}


int Transdata::Transdata_init() {
    //Register
    av_register_all();
    //Network
    avformat_network_init();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf("Could not open input file.");
        return -1;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf("Failed to retrieve input stream information");
        return -1;
    }
    videoindex = -1;
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            codecpar = ifmt_ctx->streams[i]->codecpar;
        }
    }
    //Find H.264 Decoder
    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (pCodec == NULL) {
        printf("Couldn't find Codec.\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Couldn't open codec.\n");
        return -1;
    }
    pframe = av_frame_alloc();
    if (!pframe) {
        printf("Could not allocate video frame\n");
        exit(1);
    }
    buffersrc = av_bsf_get_by_name("h264_mp4toannexb");

    if(av_bsf_alloc(buffersrc, &bsf_ctx) < 0)
        return -1;
    if(avcodec_parameters_copy(bsf_ctx->par_in,codecpar) < 0)
        return -1;
    if(av_bsf_init(bsf_ctx) < 0)
        return -1;
}



void Transdata::Yuv420p2Rgb32(const uchar *yuvBuffer_in, const uchar *rgbBuffer_out, int width, int height)
{
    uchar *yuvBuffer = (uchar *)yuvBuffer_in;
    uchar *rgb32Buffer = (uchar *)rgbBuffer_out;

    int channels = 3;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * width + x;

            int indexY = y * width + x;
            int indexU = width * height + y / 2 * width / 2 + x / 2;
            int indexV = width * height + width * height / 4 + y / 2 * width / 2 + x / 2;

            uchar Y = yuvBuffer[indexY];
            uchar U = yuvBuffer[indexU];
            uchar V = yuvBuffer[indexV];

            int R = Y + 1.402 * (V - 128);
            int G = Y - 0.34413 * (U - 128) - 0.71414*(V - 128);
            int B = Y + 1.772*(U - 128);
            R = (R < 0) ? 0 : R;
            G = (G < 0) ? 0 : G;
            B = (B < 0) ? 0 : B;
            R = (R > 255) ? 255 : R;
            G = (G > 255) ? 255 : G;
            B = (B > 255) ? 255 : B;

            rgb32Buffer[(y*width + x)*channels + 2] = uchar(R);
            rgb32Buffer[(y*width + x)*channels + 1] = uchar(G);
            rgb32Buffer[(y*width + x)*channels + 0] = uchar(B);
        }
    }
}

void Transdata::AVFrame2Img(AVFrame *pFrame, cv::Mat& img)
{
    int frameHeight = pFrame->height;
    int frameWidth = pFrame->width;
    int channels = 3;
    //输出图像分配内存
    img = cv::Mat::zeros(frameHeight, frameWidth, CV_8UC3);
    Mat output = cv::Mat::zeros(frameHeight, frameWidth,CV_8U);

    //创建保存yuv数据的buffer
    uchar* pDecodedBuffer = (uchar*)malloc(frameHeight*frameWidth * sizeof(uchar)*channels);

    //从AVFrame中获取yuv420p数据，并保存到buffer
    int i, j, k;
    //拷贝y分量
    for (i = 0; i < frameHeight; i++)
    {
        memcpy(pDecodedBuffer + frameWidth*i,
               pFrame->data[0] + pFrame->linesize[0] * i,
               frameWidth);
    }
    //拷贝u分量
    for (j = 0; j < frameHeight / 2; j++)
    {
        memcpy(pDecodedBuffer + frameWidth*i + frameWidth / 2 * j,
               pFrame->data[1] + pFrame->linesize[1] * j,
               frameWidth / 2);
    }
    //拷贝v分量
    for (k = 0; k < frameHeight / 2; k++)
    {
        memcpy(pDecodedBuffer + frameWidth*i + frameWidth / 2 * j + frameWidth / 2 * k,
               pFrame->data[2] + pFrame->linesize[2] * k,
               frameWidth / 2);
    }

    //将buffer中的yuv420p数据转换为RGB;
    Yuv420p2Rgb32(pDecodedBuffer, img.data, frameWidth, frameHeight);

    //简单处理，这里用了canny来进行二值化
//    cvtColor(img, output, CV_RGB2GRAY);
//    waitKey(2);
//    Canny(img, output, 50, 50*2);
//    waitKey(2);
    namedWindow("test", WINDOW_NORMAL);
    imshow("test",img);
    waitKey(1);
    // 测试函数
    // imwrite("test.jpg",img);
    //释放buffer
    free(pDecodedBuffer);
    //img.release();
    output.release();
}

