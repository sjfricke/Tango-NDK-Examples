package com.spencerfricke.draco_and_ply;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

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

    private View.OnClickListener getPointCloudListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            String pointCloudData = TangoJniNative.getPointCloud();

            TextView displayText = (TextView)findViewById(R.id.display_text);
            String frameText = String.format("Saved to: %s", pointCloudData);
            displayText.setText(frameText);
        }
    };


}
