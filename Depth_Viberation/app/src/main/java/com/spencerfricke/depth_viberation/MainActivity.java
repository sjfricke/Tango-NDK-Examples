package com.spencerfricke.depth_viberation;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Vibrator;

public class MainActivity extends Activity {

    Vibrator vibrator;

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

        // Need to activate the Vibrator service before it can be used
        vibrator = (Vibrator)getSystemService(VIBRATOR_SERVICE);
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

    @Override
    protected void onDestroy() {
        super.onDestroy();
        TangoJniNative.onDestroy();
    }

    // Creates viberation pulse
    // interval is the length of the viberation and dot is time between
    // A higher dot results in a slower pulse
    public void pulse(int dot) {
        int interval = 100;
        long[] pattern = {
                dot, interval, dot, interval,
                dot, interval, dot, interval,
                dot, interval, dot, interval,
                dot, interval, dot, interval,
                dot, interval, dot, interval,
                dot, interval, dot, interval,
                dot, interval, dot, interval,
                dot, interval, dot, interval
        };
        vibrator.vibrate(pattern, -1);

        // Also be aware if this callback updates the UI it can crash due to
        // having too many threads calling at once so use Activity.runOnUiThread
    }
}
