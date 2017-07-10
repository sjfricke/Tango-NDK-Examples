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
import android.widget.EditText;
import android.widget.TextView;

/**
 * This activity is responsible to hooking to the android lifecycle events to
 * native code code which calls into Tango C API.
 */
public class MainActivity extends Activity  {

    EditText serverIP;
    TextView displayText;

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

        // Needed or else the camera might not automatically grant permissions
        if (checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.CAMERA}, MY_CAMERA_REQUEST_CODE);
        }

        TangoJniNative.onCreate(this);

        Button GetPosition_btn = (Button)findViewById(R.id.Save_Button);
        GetPosition_btn.setOnClickListener(SaveButtonListener);

        Button GetPointCloud_btn = (Button)findViewById(R.id.Send_Button);
        GetPointCloud_btn.setOnClickListener(SendButtonListener);

        displayText = (TextView)findViewById(R.id.display_text);
        serverIP = (EditText)findViewById(R.id.serverIP);
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

    private View.OnClickListener SaveButtonListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            // Gets file path where app can save external files too
            String SavePath = TangoJniNative.savePNG(getApplicationContext().getExternalFilesDir(null).getAbsolutePath());

            String frameText = String.format("Saved to: %s", SavePath);
            displayText.setText(frameText);
        }
    };

    private View.OnClickListener SendButtonListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            String ip = serverIP.getText().toString();

            int status = TangoJniNative.sendPNG(ip);

            if (status >= 0) {
                String frameText = String.format("PNG Sent to server!");
                displayText.setText(frameText);
            } else {
                String frameText = String.format("Failed sending to server with error: %d", status);
                displayText.setText(frameText);
            }
        }
    };

}
