package com.spencerfricke.ui_interface;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

/**
 * This activity is responsible to hooking to the android lifecycle events to
 * native code code which calls into Tango C API.
 */
public class MainActivity extends Activity  {

    // Tango Service connection.
    ServiceConnection mTangoServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            // Synchronization around MainActivity object is to avoid
            // Tango disconnect in the middle of the connecting operation.
            TangoJniNative.onTangoServiceConnected(service);
        }

        public void onServiceDisconnected(ComponentName name) {
            // Handle this if you need to gracefully shutdown/retry
            // in the event that Tango itself crashes/gets upgraded while running.
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TangoJniNative.onCreate(this);

        Button GetPosition_btn = (Button)findViewById(R.id.Position_Button);
        GetPosition_btn.setOnClickListener(getPostionListener);

        Button GetFrameImage_btn = (Button)findViewById(R.id.Image_Frame);
        GetFrameImage_btn.setOnClickListener(getFrameImageListener);

        Button GetPointCloud_btn = (Button)findViewById(R.id.Point_Cloud);
        GetPointCloud_btn.setOnClickListener(getPointCloudListener);
    }

    @Override
    protected void onResume() {
        super.onResume();
        TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection);
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Disconnect from Tango Service, release all the resources that the app is
        // holding from Tango Service.
        TangoJniNative.onPause();
        unbindService(mTangoServiceConnection);
    }

    double[] poseData = new double[7];

    private View.OnClickListener getPostionListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            poseData = TangoJniNative.getPosition();

            TextView displayText = (TextView)findViewById(R.id.display_text);
            String poseText = String.format("Position\n X: %.3f\nY: %.3f\nZ: %.3f\n\n" +
                                            "Orientation\nX: %.3f\nY: %.3f\nZ: %.3f\nW: %.3f",
                    poseData[0], poseData[1], poseData[2],
                    poseData[3], poseData[4], poseData[5], poseData[6]);
            displayText.setText(poseText);
        }
    };

    long[] frameData = new long[5];

    private View.OnClickListener getFrameImageListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            frameData = TangoJniNative.getFrameImage();

            TextView displayText = (TextView)findViewById(R.id.display_text);
            String frameText = String.format("Height: %d\nWidth: %d\nStride: %d\nExposure Duration: %d ns\nFrame Number: %d",
                    frameData[0], frameData[1], frameData[2], frameData[3], frameData[4]);
            displayText.setText(frameText);
        }
    };

    private View.OnClickListener getPointCloudListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            int pointCloudData = TangoJniNative.getPointCloud();

            TextView displayText = (TextView)findViewById(R.id.display_text);
            String frameText = String.format("Point Cloud Count: %d", pointCloudData);
            displayText.setText(frameText);
        }
    };

}
