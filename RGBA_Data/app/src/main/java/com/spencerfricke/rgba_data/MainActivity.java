package com.spencerfricke.rgba_data;

import android.Manifest;
import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
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

    private static final int MY_CAMERA_REQUEST_CODE = 100;

    private ServiceConnection mTangoServiceCoonnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            TangoJniNative.onTangoServiceConnected(binder);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            // Handle this if you need to gracefully shutdown/retry
            // in the event that Tango itself crashes/gets upgraded while running.
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        if (checkSelfPermission(Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.CAMERA},
                    MY_CAMERA_REQUEST_CODE);
        }

        TangoJniNative.onCreate(this);
    }

    @Override
    protected void onResume() {
        super.onResume();

        TangoInitializationHelper.bindTangoService(this, mTangoServiceCoonnection);

    }

    @Override
    protected void onPause() {
        super.onPause();
        TangoJniNative.onPause();
        unbindService(mTangoServiceCoonnection);
    }

//    // Tango Service connection.
//    ServiceConnection mTangoServiceConnection = new ServiceConnection() {
//        public void onServiceConnected(ComponentName name, IBinder service) {
//            // Synchronization around MainActivity object is to avoid
//            // Tango disconnect in the middle of the connecting operation.
//            TangoJniNative.onTangoServiceConnected(service);
//        }
//
//        public void onServiceDisconnected(ComponentName name) {
//            // Handle this if you need to gracefully shutdown/retry
//            // in the event that Tango itself crashes/gets upgraded while running.
//        }
//    };
//
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);
//        TangoJniNative.onCreate(this);
//
//        Button GetPosition_btn = (Button)findViewById(R.id.Position_Button);
//        GetPosition_btn.setOnClickListener(getPostionListener);
//
//        Button GetPointCloud_btn = (Button)findViewById(R.id.Point_Cloud);
//        GetPointCloud_btn.setOnClickListener(getPointCloudListener);
//    }
//
//    @Override
//    protected void onResume() {
//        super.onResume();
//        TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection);
//    }
//
//    @Override
//    protected void onPause() {
//        super.onPause();
//        // Disconnect from Tango Service, release all the resources that the app is
//        // holding from Tango Service.
//        TangoJniNative.onPause();
//        unbindService(mTangoServiceConnection);
//    }
//
//    private View.OnClickListener getPostionListener = new View.OnClickListener() {
//        @Override
//        public void onClick(View view) {
//
//        }
//    };
//
//    private View.OnClickListener getPointCloudListener = new View.OnClickListener() {
//        @Override
//        public void onClick(View view) {
//
//        }
//    };

}
