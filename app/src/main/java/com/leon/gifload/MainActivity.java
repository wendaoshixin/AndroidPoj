package com.leon.gifload;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.resource.drawable.GlideDrawable;
import com.bumptech.glide.request.animation.GlideAnimation;
import com.bumptech.glide.request.target.GlideDrawableImageViewTarget;
import com.cunoraz.gifview.library.GifView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String TAG = "GifLoadMainActivity";
    protected Button btn_play;
    protected Button btn_pause;
    protected Button btn_play_gifview;

    protected ImageView image;
    private Bitmap bitmap;
    private GifHandler gifHandler;
    private GifView gifView1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btn_play = (Button) findViewById(R.id.btn_play);
        btn_pause = (Button) findViewById(R.id.btn_pause);
        btn_play_gifview = (Button) findViewById(R.id.btn_play_gifview);

        image = (ImageView) findViewById(R.id.image);
        btn_play.setOnClickListener(this);
        btn_pause.setOnClickListener(this);
        btn_play_gifview.setOnClickListener(this);

        gifView1 = (GifView) findViewById(R.id.gif1);
        gifView1.setVisibility(View.VISIBLE);

        //拷贝gif图片到sd卡
        new Thread() {
            @Override
            public void run() {
                InputStream is = getResources().openRawResource(R.raw.jump);
                File file = new File(Environment.getExternalStorageDirectory(), "jump.gif");
                OutputStream out = null;
                try {
                    out = new FileOutputStream(file);
                    byte[] buf = new byte[1024];
                    int len;
                    while ((len = is.read(buf, 0, buf.length)) != -1) {
                        out.write(buf, 0, len);
                    }
                    Log.e("leon-tag", "finish copy");
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    try {
                        if (is != null) {
                            is.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    try {
                        if (out != null) {
                            out.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }.start();

    }

    @Override
    public void onClick(View v) {
        if (v == btn_pause) {
            pause = true;
        } else if (v == btn_play) {
            pause = false;
            if (gifHandler == null) {
                File file = new File(Environment.getExternalStorageDirectory(), "test.gif");
                gifHandler = GifHandler.load(file.getAbsolutePath());
                bitmap = Bitmap.createBitmap(gifHandler.getGifWidth()
                        , gifHandler.getGifHeight(), Bitmap.Config.ARGB_8888);
            }
            int frame_duration = gifHandler.updateFrame(bitmap);
            image.setImageBitmap(bitmap);
            handler.sendEmptyMessageDelayed(1, frame_duration);
        } else if (v == btn_play_gifview) {
//            gifView1.setGifResource(R.raw.test);
//            gifView1.play();
//            gifView1.pause();
//            gifView1.setMovieTime(time);
//            gifView1.getMovie();

            Glide.with(getApplicationContext()).load(R.raw.test).into(new GlideDrawableImageViewTarget(image,1024){
                @Override
                public void onResourceReady(GlideDrawable resource, GlideAnimation<? super GlideDrawable> animation) {
                    super.onResourceReady(resource, animation);
                    Log.e(TAG, "onResourceReady:加载完成");
                }

                @Override
                public void onLoadFailed(Exception e, Drawable errorDrawable) {
                    super.onLoadFailed(e, errorDrawable);
                    Log.e(TAG, "onLoadFailed:" );
                }

                @Override
                public void onLoadStarted(Drawable placeholder) {
                    super.onLoadStarted(placeholder);
                    Log.e(TAG, "onLoadStarted:" );
                }

                @Override
                public void onStart() {
                    super.onStart();
                    Log.e(TAG, "onStart:" );
                }

                @Override
                public void onStop() {
                    super.onStop();
                    Log.e(TAG, "onStop:" );
                }
            });
        }

    }

    private boolean pause = false;
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (pause) {
                return;
            }
            int frame_duration = gifHandler.updateFrame(bitmap);
            image.setImageBitmap(bitmap);
            handler.sendEmptyMessageDelayed(1, frame_duration);
        }
    };
}
