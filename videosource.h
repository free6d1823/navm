#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H
#include "stdio.h"

#define MAX_BUFFERS 2
#define STATE_FREE  0
#define STATE_WRITEN    1
#define STATE_READ      2

typedef struct tagFrameInfo{
    unsigned char* pData;    //pointer to data
    long time;      //image grabbed time
    int state;       //1: buffer is writing 2:buffer is reading 0:free
}FrameInfo;

class VideoSource
{
public:
    VideoSource();
    ~VideoSource();

    int Width(){return m_nWidth;}
    int Height() {return m_nHeight;}
    unsigned char * GetFrameData();
    void* ProcessFrame();

protected:
    bool init();

    unsigned char* m_pDataYuv;//YUYV temp buffer

    int m_nWidth;
    int m_nHeight;
    int m_nBytesPerLine;
    //ping-pong buffers
    unsigned char* m_pData;//RGB888
    FrameInfo m_frameInfo[MAX_BUFFERS];


};

#endif // VIDEOSOURCE_H
