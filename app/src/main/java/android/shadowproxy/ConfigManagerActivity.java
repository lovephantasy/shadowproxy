package android.shadowproxy;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.shadowproxy.ListViewEx.ConfigAdapter;
import android.shadowproxy.ListViewEx.ConfigHolder;
import android.shadowproxy.ListViewEx.ConfigItem;
import android.shadowproxy.ListViewEx.ListViewEx;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.Toast;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public class ConfigManagerActivity extends Activity {

    private ListViewEx listView=null;

    private ConfigAdapter adapter=null;
    SharedPreferences mSharedPreference=null;
    public static String POSTFIX=".conf";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_configmanager);
        setupActionBar();
        setupListView();
        new GetConfigs().execute();
    }
    private void setupListView(){
        listView = (ListViewEx) findViewById(R.id.lv_cm);

        adapter = new ConfigAdapter(this);
        adapter.setOnSelectListener(new ConfigAdapter.OnSelecListener() {
            @Override
            public void select(int position,int lastsel) {
                if (mSharedPreference == null) {
                    mSharedPreference = getSharedPreferences(getString(R.string.app_name), MODE_PRIVATE);
                }
                mSharedPreference.edit().putString("current", adapter.getDatas().get(position).mFilename+POSTFIX).apply();
                View v= listView.getChildAt(lastsel-listView.getFirstVisiblePosition());
                if (v != null) {
                    ConfigHolder h= (ConfigHolder) v.getTag();
                    h.bt_select.setEnabled(true);
                }else {
                    adapter.notifyDataSetChanged();
                }
                ((ConfigHolder) listView.getChildAt(position-listView.getFirstVisiblePosition()).getTag()).bt_select.setEnabled(false);
            }
        });

        adapter.setOnDelListener(new ConfigAdapter.OnDelListener() {
            @Override
            public void del(final int position) {
                new AlertDialog.Builder(ConfigManagerActivity.this).setMessage("确认删除 "+adapter.getDatas().get(position).mFilename+"?")
                        .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                String path=Environment.getExternalStorageDirectory().getAbsolutePath()+"/"+getString(R.string.app_name) +"/"+adapter.getDatas().get(position).mFilename+POSTFIX;

                                if (new File(path).delete()) {
                                    adapter.remove(position);
                                    Toast.makeText(ConfigManagerActivity.this, "删除成功", Toast.LENGTH_SHORT).show();
                                }
                            }
                        }).show();

            }
        });
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String filename =  Environment.getExternalStorageDirectory().getAbsolutePath()+"/"+getString(R.string.app_name) +"/"+adapter.getDatas().get(position).mFilename+POSTFIX;
                Intent i = new Intent(ConfigManagerActivity.this,EditActivity.class);
                i.putExtra("Filename", filename);
                startActivity(i);

            }
        });
    }

    private class GetConfigs extends AsyncTask<Void, Void, List<ConfigItem>> {
        //ProgressDialog progressDialog=null;
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
           // progressDialog = Utils.getProcessDialog(ConfigManagerActivity.this);
          //  progressDialog.show();
        }

        @Override
        protected List<ConfigItem> doInBackground(Void... params) {
            if (mSharedPreference == null) {
                mSharedPreference=getSharedPreferences(getString(R.string.app_name), MODE_PRIVATE);
            }
            String current = mSharedPreference.getString("current", "null");
            File file=new File(Environment.getExternalStorageDirectory().getAbsolutePath()+"/"+getString(R.string.app_name));
            if (!file.exists()) {
                if (!file.mkdirs()) {
                    Toast.makeText(ConfigManagerActivity.this,"读取SDCARD失败，请确认是否授予存储权限！",Toast.LENGTH_SHORT).show();
                    return null;
                }
            }
            String[] list = file.list();
            List<ConfigItem> tmp = new ArrayList<>();
            for (String s : list) {
                if (s.endsWith(POSTFIX)) {
                    boolean b=current.equals(s);
                    tmp.add(new ConfigItem(s.substring(0,s.lastIndexOf(".")),b));
                }
            }
            try {
                Thread.sleep(300);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return tmp;
        }

        @Override
        protected void onPostExecute(List<ConfigItem> items) {
            super.onPostExecute(items);
           // progressDialog.dismiss();
            if (items != null) {
                adapter.addAll(items);
            }

        }
    }

    private void setupActionBar() {
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setTitle("文件");

            // Show the Up button in the action bar.
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.manager,menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            finish();
            return true;
        } else if (id == R.id.menu_new) {
            final EditText editText = new EditText(ConfigManagerActivity.this);
            new AlertDialog.Builder(ConfigManagerActivity.this).setView(editText).setTitle("输入文件名").setPositiveButton("确定", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    String filename = editText.getText().toString();
                    String path=Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + getString(R.string.app_name) + "/" + filename+POSTFIX;
                    File file = new File(path);
                    if (file.exists()) {
                        Toast.makeText(ConfigManagerActivity.this, "文件已存在", Toast.LENGTH_SHORT).show();
                    }else {
                        try {
                            if(file.createNewFile()){
                                adapter.add(new ConfigItem(filename,false));
                                startActivity(new Intent(ConfigManagerActivity.this,EditActivity.class).putExtra("Filename",path));

                            }else {
                                Toast.makeText(ConfigManagerActivity.this, "创建文件失败", Toast.LENGTH_SHORT).show();

                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }

                    }

                }
            }).show();



        } else if (id == R.id.menu_input) {

        } else if (id == R.id.menu_refresh) {
                adapter.removeAll();
                 new GetConfigs().execute();

        }

        return super.onOptionsItemSelected(item);
    }

}
