package android.shadowproxy;

import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.VpnService;
import android.os.Build;
import android.os.Environment;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;

import java.io.File;


public class ShadowProxy extends VpnService implements Runnable {

    static {
        System.loadLibrary("core");
    }

    public native void initCore();

    public native void startVPN_jni(int fd,String path);

    public native void stopVPN_jni();

    public static ShadowProxy Instance;


    private Thread mVPNThread;
    private ParcelFileDescriptor mVPNInterface;

    static private onStatusChangedListener m_listener = null;

    public interface onStatusChangedListener {
        void onRunning(boolean isRunning);

        void onChanged(String msg);
    }

    static public void setOnStatusChangedListener(onStatusChangedListener listener) {
        m_listener = listener;
    }

    public ShadowProxy() {
        Instance = this;
    }


    @Override
    public void onCreate() {
        LogTools.e("vpn", "onCreate");

        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        LogTools.e("vpn", "onStartCommand");

        // Start a new session by creating a new thread.
        mVPNThread = new Thread(this, "VPNService");

        mVPNThread.start();
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        LogTools.e("vpn", "onBind");

        String action = intent.getAction();
        if (action.equals(VpnService.SERVICE_INTERFACE)) {
            return super.onBind(intent);
        }
        return null;
    }


    @Override
    public synchronized void run() {
        try {
            LogTools.e(App.getContext().getString(R.string.app_name), "Service start running");

            waitUntilPreapred();

            runVPN();

        } catch (InterruptedException e) {
            LogTools.e(App.getContext().getString(R.string.app_name), "Exception", e);
        } catch (Exception e) {
            e.printStackTrace();
        }

        //dispose();
    }

    private void runVPN() throws Exception {
        /**
         * jni init
         */
        initCore();
        /**
         * get tun fd
         */
        String n=getConfigPathName();
        if ((this.mVPNInterface = buildVpnTun()) != null&&n!=null) {
            /**
             * jni start
             */
            if (m_listener != null)
                m_listener.onRunning(true);
            startVPN_jni(mVPNInterface.getFd(),n);
        } else {
            if (m_listener != null)

                m_listener.onRunning(false);
        }


    }

    private String getConfigPathName(){
       SharedPreferences s = getSharedPreferences(getString(R.string.app_name), MODE_PRIVATE);
        String file = s.getString("current", "null");
        String filepath= Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + getString(R.string.app_name) + "/" +file;
        if (file.equals("null")|!new File(filepath).exists()){
            return null;
        }
        return filepath;
    }

    private void waitUntilPreapred() {
        while (prepare(this) != null) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
                // Ignore
            }
        }
    }

    private ParcelFileDescriptor buildVpnTun() throws Exception {
        Builder builder = new Builder();
        builder.setMtu(1500);

        builder.addAddress("10.0.0.86", 32);

        builder.addDnsServer("114.114.114.114");

        for (String routeAddress : getResources().getStringArray(R.array.bypass_private_route)) {
            String[] addr = routeAddress.split("/");
            builder.addRoute(addr[0], Integer.parseInt(addr[1]));
        }
        // builder.addRoute("0.0.0.0", 0);

        Intent intent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, 0);
        builder.setConfigureIntent(pendingIntent);


        if (Build.VERSION.SDK_INT>=Build.VERSION_CODES.LOLLIPOP){
            builder.setBlocking(true);
        }
        return builder.establish();


    }

    public void stopVPN() {
        /**
         * stop jni services
         */
        stopVPN_jni();
        /**
         * close fd,threds
         */
        dispose();
        /**
         * force stop self
         */
        stopSelf();
    }

    private void dispose() {

        try {
            if (mVPNInterface != null) {
                mVPNInterface.close();
                mVPNInterface = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
            // ignore
        }


        if (mVPNThread != null) {
            mVPNThread.interrupt();
            mVPNThread = null;
        }
    }

    @Override
    public void onDestroy() {
        LogTools.e(App.getContext().getString(R.string.app_name), "Service destroyed");
        if (m_listener != null)
            m_listener.onRunning(false);
        Instance=null;
        dispose();

        super.onDestroy();
    }

}
