package com.leon.gifload;

import android.graphics.Bitmap;

/**
 * Gif加载类
 *
 * @author 何敏
 *         Date 2017/5/16
 */
public class GifHandler {
    private long gifPoint;

    static {
        System.loadLibrary("native-lib");
    }

    public GifHandler(long gifPoint) {
        this.gifPoint = gifPoint;
    }

    public static native long loadGif(String path);

    public static native int getWidth(long gifPoint);

    public static native int getHeight(long gifPoint);

    public static native int getNextTime(long gifPoint);

    public static native int updateFrame(long gifPoint, Bitmap bitmap);

    public static GifHandler load(String path) {
        long gifHandlerPoint = loadGif(path);
        GifHandler gifHandler = new GifHandler(gifHandlerPoint);
        return gifHandler;
    }

    public int getGifWidth(){
        return getWidth(gifPoint);
    }

    public int getGifHeight(){
        return getHeight(gifPoint);
    }

    public int updateFrame(Bitmap bitmap){
        return updateFrame(gifPoint,bitmap);
    }
}
