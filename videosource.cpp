#include "videosource.h"
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>


pthread_t m_thread = (pthread_t)NULL;
bool       m_bTerminate = false;
pthread_mutex_t m_lock;


#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)


// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)


void Yuv411ToRgb32(unsigned char* pYuv, int width, int height, unsigned char* pRgb)
{
    int nBps = width*4;
    unsigned char* pY1 = pYuv;
    unsigned char* pY2 = pYuv + width;
//    unsigned char* pV = pYuv + width*height;
//    unsigned char* pU = pV + width*height/4;
    unsigned char* pU = pYuv + width*height;
    unsigned char* pV = pU + width*height/4;

    unsigned char* pLine1 = pRgb;
    unsigned char* pLine2 = pLine1 + nBps;

    unsigned char y1,u,v,y2;
    for (int i=0; i<height; i+=2)
    {
        for (int j=0; j<width; j+=2)
        {
            y1 = pY1[j];
            y2 = pY2[j];
            u = pU[j/2];
            v = pV[j/2];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;
            pLine2[j*4] = YUV2B(y2, u, v);//b
            pLine2[j*4+1] = YUV2G(y2, u, v);//g
            pLine2[j*4+2] = YUV2R(y2, u, v);//r
            pLine2[j*4+3] = 0xff;

            y1 = pY1[j+1];
            y2 = pY2[j+1];
            pLine1[j*4+4] = YUV2B(y1, u, v);//b
            pLine1[j*4+5] = YUV2G(y1, u, v);//g
            pLine1[j*4+6] = YUV2R(y1, u, v);//r
            pLine1[j*4+7] = 0xff;
            pLine2[j*4+4] = YUV2B(y2, u, v);//b
            pLine2[j*4+5] = YUV2G(y2, u, v);//g
            pLine2[j*4+6] = YUV2R(y2, u, v);//r
            pLine2[j*4+7] = 0xff;
        }
        pY1 = pY2 + width;
        pY2 = pY1 + width;
        pU += width/2;
        pV += width/2;
        pLine1 = pLine2 + nBps;
        pLine2 = pLine1 + nBps;

    }
}

static int m_nWriteIndex = 0;

//filse simulation
static int m_nTotalFrames = 0;
static int m_nNextFrame = -1;
static FILE*   m_pFile = NULL;
static int m_nBytesPerFrame; //yuv frame


VideoSource::VideoSource()
{
    m_nWidth = 1024;
    m_nHeight = 1024;
    m_nBytesPerLine = 1024*3/2;//YUYV

    m_nBytesPerFrame = m_nBytesPerLine* m_nHeight;
    m_pDataYuv = (unsigned char*) malloc(m_nWidth* m_nHeight*3/2*MAX_BUFFERS);
    m_pData = (unsigned char*) malloc(m_nWidth*4* m_nHeight);
    for (int i=0; i< MAX_BUFFERS; i++) {
        m_frameInfo[i].pData = m_pDataYuv + m_nBytesPerFrame;
        m_frameInfo[i].state = 0;
        m_frameInfo[i].time = 0;
    }
    m_nWriteIndex = 0; //next frame to write
    init();
}

VideoSource::~VideoSource()
{
    m_bTerminate = true;
    if(m_thread) {
        pthread_join(m_thread, NULL);
        pthread_mutex_destroy(&m_lock);
    }
    if(m_pData) free(m_pData);
    if(m_pDataYuv) free(m_pDataYuv);
    if(m_pFile) {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}


unsigned char * VideoSource::GetFrameData()
{
    pthread_mutex_lock(&m_lock);
    int indexR = (m_nWriteIndex -1);
    if (indexR < 0)indexR = MAX_BUFFERS-1;
    if (m_frameInfo[indexR].state != STATE_WRITEN){
        printf("Warring: no buffer [%d] state=%d!\n", indexR, m_frameInfo[indexR].state);
    } else {

        Yuv411ToRgb32(m_frameInfo[indexR].pData, m_nWidth, m_nHeight,
                m_pData);

        m_frameInfo[indexR].state = STATE_FREE;
    }
    pthread_mutex_unlock(&m_lock);
    return m_pData;
}


void* DoGetFrame(void* data)
{
    VideoSource* pThis = (VideoSource*) data;
    return pThis->ProcessFrame();
}

void* VideoSource::ProcessFrame()
{
    if (m_pFile == NULL) {
        printf("File not opened, please call init first!\n");
        return (void*) 0;
    }

    while(!m_bTerminate) {


        pthread_mutex_lock(&m_lock);

        if(m_frameInfo[m_nWriteIndex].state != STATE_READ){
            //file simulation
            fseek(m_pFile, m_nNextFrame*m_nBytesPerFrame , SEEK_SET);
            if (1 == fread(m_frameInfo[m_nWriteIndex].pData, m_nBytesPerFrame, 1, m_pFile)){
                m_frameInfo[m_nWriteIndex].state = STATE_WRITEN;
                m_frameInfo[m_nWriteIndex].time ++; //just for ref
                m_nWriteIndex ++;
                if (m_nWriteIndex >= MAX_BUFFERS)m_nWriteIndex = 0;

                m_nNextFrame++;
                if (m_nNextFrame >= m_nTotalFrames)
                    m_nNextFrame = 0;

            }else {
                printf("read file error!!\n");
            }
            //
        }
        pthread_mutex_unlock(&m_lock);
        usleep(30000);
    }

    return (void*) 0;
}
bool VideoSource::init()
{
    m_pFile = fopen("/opt/out128.yuv", "rb");
    if (!m_pFile) {
        printf("Open file error!\n");
        return false;
    }
    m_nNextFrame = 0;
    fseek(m_pFile, 0, SEEK_END);
    long length = ftell(m_pFile);
    m_nTotalFrames = length/ m_nBytesPerFrame;

    pthread_mutex_init(&m_lock,NULL);
    pthread_create(&m_thread, NULL, DoGetFrame, (void*) this);

    return true;
}
