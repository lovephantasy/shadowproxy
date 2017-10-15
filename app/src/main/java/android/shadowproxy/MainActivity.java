package android.shadowproxy;

import android.Manifest;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {


    Button bt_start;
    Button bt_stop;
    ActionBar actionBar;
    private SharedPreferences s = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        actionBar = getActionBar();
        if (actionBar!=null) {
            actionBar.setDisplayShowHomeEnabled(true);
            actionBar.setDisplayShowTitleEnabled(true);
        }

        s = getSharedPreferences(getString(R.string.app_name), MODE_PRIVATE);

        new InitTask().execute();

        bt_start = (Button) findViewById(R.id.bt_start);
        bt_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
              //  bt_start.setEnabled(false);
                String file = s.getString("current", "null");
                String filepath=Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + getString(R.string.app_name) + "/" +file;
                if (file.equals("null")|!new File(filepath).exists()){
                    Toast.makeText(MainActivity.this,"请先选择配置文件！",Toast.LENGTH_SHORT).show();
                    return;
                }
                if (ShadowProxy.Instance != null)return;
                    Intent intent = ShadowProxy.prepare(MainActivity.this);
                if (intent == null) {
                    startVpn();
                    setVpnListener();
                } else {
                    startActivityForResult(intent, 1);
                }
            }
        });

        bt_stop = (Button) findViewById(R.id.bt_stop);
        bt_stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (ShadowProxy.Instance != null)
                    ShadowProxy.Instance.stopVPN();
            }
        });
    }

    private void setVpnListener() {
        if (ShadowProxy.Instance != null) {
            ShadowProxy.setOnStatusChangedListener(new ShadowProxy.onStatusChangedListener() {
                @Override
                public void onRunning(boolean isRunning) {
                    if (isRunning) {
                        //  bt_start.setEnabled(false);

                    } else {
                        bt_start.setEnabled(true);


                    }
                }

                @Override
                public void onChanged(String msg) {
                    Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
                }
            });
        }
    }


    void startVpn() {

        startService(new Intent(this, ShadowProxy.class));
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 1) {
            if (resultCode == RESULT_OK) {
                startVpn();
                setVpnListener();
            } else {
              //  bt_start.setEnabled(true);
                actionBar.setTitle("开启失败");
            }
        }

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return super.onCreateOptionsMenu(menu);
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_item_config:
                startActivity(new Intent(this, ConfigManagerActivity.class));
                break;
            case R.id.menu_item_settting:
                startActivity(new Intent(this, SettingsActivity.class));
                break;
            default:
                break;

        }
        return super.onOptionsItemSelected(item);
    }
    private class InitTask extends AsyncTask<Void, Void, Void> {
        ProgressDialog progressDialog = null;
        boolean firstrun = false;

        @Override
        protected void onPreExecute() {

            progressDialog = new ProgressDialog(MainActivity.this);
            progressDialog.setMessage("初始化中...");
            progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            progressDialog.setIndeterminate(false);
            progressDialog.setCancelable(false);
            progressDialog.show();
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            progressDialog.setMessage("完成！");
            progressDialog.dismiss();
            if (firstrun)
                initPermission();
        }

        @Override
        protected Void doInBackground(Void... params) {

            try {
                //mkdirs();  未检查权限
                SharedPreferences s = getSharedPreferences(getString(R.string.app_name), MODE_PRIVATE);
                if (s.getBoolean(getString(R.string.firstrun), true)) {
                    // initCProxy();
                    firstrun = true;
                    initPreferences(s);
                    s.edit().putBoolean(getString(R.string.firstrun), false).apply();
                    Thread.sleep(1000);
                }

            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }


        private void initPermission() {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                String permissions[] = {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE};
                List<String> permisssions1 = new ArrayList<>();
                for (String p : permissions) {
                    if (checkSelfPermission(p) != PackageManager.PERMISSION_GRANTED) {
                        permisssions1.add(p);
                    }
                }
                if(permisssions1.size()==0)return;
                String[] array = new String[permisssions1.size()];
                requestPermissions(permisssions1.toArray(array), 0);
            } else {
                new InitSDCARD().execute();
            }
        }

        private void initPreferences(SharedPreferences s) {

        }
    }

    private class InitSDCARD extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... params) {
            mkdirs();
            example();
            return null;
        }

        private void example() {
            File file = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + getString(R.string.app_name) + "/" + "example"+ConfigManagerActivity.POSTFIX);
            if (!file.exists()) {
                try {
                    file.createNewFile();
                    FileOutputStream fos = new FileOutputStream(file);
                    fos.write(ConfigExample.example.getBytes());
                    fos.flush();
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        private void mkdirs() {
            File dirs = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + getString(R.string.app_name));
            if (dirs.exists()) return;
            if (!dirs.mkdirs()) {
                Toast.makeText(MainActivity.this, "创建文件夹失败！", Toast.LENGTH_SHORT).show();
            }


        }
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == 0) {
            for (int i = 0; i < grantResults.length; i++) {
                if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                    if (!shouldShowRequestPermissionRationale(permissions[i])) {
                        AlertDialog dialog = new AlertDialog.Builder(this)
                                .setTitle("授权失败")
                                .setMessage(permissions[i])
                                .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        //finish();
                                    }
                                }).create();
                        dialog.show();
                    }
                } else {
                    //LogTools.e("test", permissions[i]);
                    if (permissions[i].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                        // LogTools.e("test", Environment.getExternalStorageDirectory().getAbsolutePath() + "/VPN4CProxy/Error");
                        new InitSDCARD().execute();
                    }
                }


            }
        }
    }
}

