#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <stdlib.h>
#include <stdio.h>
#include "gif_lib.h"

#define LOG_TAG "leon-tag"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__);

#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)
#define  delay(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))
#define  dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  transparency(ext) ((ext)->Bytes[0] & 1)
#define  trans_index(ext) ((ext)->Bytes[3])

typedef struct GifBean {
    //总时间
    int total_duration;
    //当前帧
    int current_frame;
    //每一帧的时间
    int frame_duration;
    //总共多少帧
    int total_frame;
} GifBean;
extern "C" {

void drawFrame(GifFileType* gif, AndroidBitmapInfo*  info, int* pixels, int frame_no, bool force_dispose_1) {
    GifColorType *bg;
    GifColorType *color;
    SavedImage * frame;
    ExtensionBlock * ext = 0;
    GifImageDesc * frameInfo;
    ColorMapObject * colorMap;
    int *line;
    int width, height,x,y,j,loc,n,inc,p;
    int* px;

    width = gif->SWidth;
    height = gif->SHeight;


    frame = &(gif->SavedImages[frame_no]);
    frameInfo = &(frame->ImageDesc);
    if (frameInfo->ColorMap) {
        colorMap = frameInfo->ColorMap;
    } else {
        colorMap = gif->SColorMap;
    }

    bg = &colorMap->Colors[gif->SBackGroundColor];

    for (j=0; j<frame->ExtensionBlockCount; j++) {
        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
            ext = &(frame->ExtensionBlocks[j]);
            break;
        }
    }

    // For dispose = 1, we assume its been drawn
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose_1 && frame_no > 0) {
        drawFrame(gif, info, pixels, frame_no-1, true);
    }
    else if (ext && dispose(ext) == 2 && bg) {
        for (y=0; y<height; y++) {
            line = (int*) px;
            for (x=0; x<width; x++) {
                line[x] = argb(255, bg->Red, bg->Green, bg->Blue);
            }
            px = (int *) ((char*)px + info->stride);
        }
    } else if (ext && dispose(ext) == 3 && frame_no > 1) {
        drawFrame(gif, info, pixels, frame_no-2, true);
    }
    px = (int *) ((char*)px + info->stride * frameInfo->Top);
    for (y=frameInfo->Top; y<frameInfo->Top+frameInfo->Height; y++) {
        line = (int*) px;
        for (x=frameInfo->Left; x<frameInfo->Left+frameInfo->Width; x++) {
            loc = (y - frameInfo->Top)*frameInfo->Width + (x - frameInfo->Left);
            if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
                continue;
            }
            color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];
            if (color)
                line[x] = argb(255, color->Red, color->Green, color->Blue);
        }
        px = (int *) ((char*)px + info->stride);
    }
}

JNIEXPORT jint JNICALL
Java_com_leon_gifload_GifHandler_getNextTime(JNIEnv *env, jclass type, jlong gifHandler) {
    GifFileType * gifFileType = (GifFileType *) gifHandler;
    GifBean * gifBean= (GifBean *) gifFileType->UserData;
    return gifBean->frame_duration;
}

JNIEXPORT jint JNICALL
Java_com_leon_gifload_GifHandler_updateFrame(JNIEnv *env, jclass type, jlong gifHandler,
                                             jobject bitmap) {
    //强转代表gif图片的结构体
    GifFileType *gifFileType = (GifFileType *) gifHandler;
    GifBean *gifBean = (GifBean *) gifFileType->UserData;
    AndroidBitmapInfo info;
    //代表一幅图片的像素数组
    void * pixels;
    AndroidBitmap_getInfo(env,bitmap,&info);
    //锁定bitmap 一幅图片--》 二维数组  ==》 一个二维数组 将bitmap的像素数组存放到pixels
    AndroidBitmap_lockPixels(env,bitmap,&pixels);
    //绘制一帧
    drawFrame(gifFileType,&info,(int*)pixels,gifBean->current_frame,false);
    //当前帧加一
    gifBean->current_frame+=1;
    //一旦绘制到最后一帧，回到第一帧
    if(gifBean->current_frame>=gifBean->total_frame){
        gifBean->current_frame=0;
    }
    //解锁画布
    AndroidBitmap_unlockPixels(env,bitmap);

    return gifBean->frame_duration;
}



JNIEXPORT jint JNICALL
Java_com_leon_gifload_GifHandler_getHeight(JNIEnv *env, jclass type, jlong gifHandler) {
    GifFileType * gifFileType = (GifFileType *) gifHandler;
    return gifFileType->SHeight;
}


JNIEXPORT jint JNICALL
Java_com_leon_gifload_GifHandler_getWidth(JNIEnv *env, jclass type, jlong gifHandler) {
    GifFileType * gifFileType = (GifFileType *) gifHandler;
    return gifFileType->SWidth;
}


char *jstringTostring(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);

    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}


JNIEXPORT jlong JNICALL
Java_com_leon_gifload_GifHandler_loadGif(JNIEnv *env, jclass type, jstring path_) {
    char *filePath = jstringTostring(env, path_);
    LOGE("filePath %s", filePath);
    int err;
    //打开gif文件
    GifFileType *gifFileType = DGifOpenFileName(filePath, &err);
    //初始化结构体
    DGifSlurp(gifFileType);
    GifBean *gifBean = (GifBean *) malloc(sizeof(GifBean));
    gifBean->frame_duration = 0;
    gifBean->current_frame = 0;
    gifBean->total_duration = 0;
    gifBean->total_frame = 0;
    //好比一个View 去打一个Tag
    gifFileType->UserData = gifBean;
    //下一步 给gifBean的成员变量赋值  得到当前播放时间的总时长
    int i, j, frame_delay;
    SavedImage *frame;
    ExtensionBlock *ext;
    for (i = 0; i < gifFileType->ImageCount; i++) {
        frame = &(gifFileType->SavedImages[i]);

        for (j = 0; j < frame->ExtensionBlockCount; j++) {
            if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                ext = &(frame->ExtensionBlocks[j]);
                break;
            }
        }

        if (ext) {
            //延迟时间
            frame_delay = 10 * (ext->Bytes[2] << 8 | ext->Bytes[1]);
            LOGE("延迟时间 %d ms", frame_delay);
            //得到gif的总时间
            gifBean->total_duration += frame_delay;
        }
    }

    gifBean->frame_duration = gifBean->total_duration / gifFileType->ImageCount;
    gifBean->total_frame = gifFileType->ImageCount;
    LOGE("gif 总帧数 %d", gifFileType->ImageCount);

    return (long long) gifFileType;
}




}