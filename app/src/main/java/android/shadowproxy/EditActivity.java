package android.shadowproxy;

import android.app.ActionBar;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.Toast;

import java.io.File;

public class EditActivity extends Activity {

    EditText et_config = null;
    String filename;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_edit);
        setupActionBar();
        Intent intent = getIntent();
         filename= intent.getStringExtra("Filename");
        et_config = (EditText) findViewById(R.id.et_config);
        new GetFileContent().execute(filename);

    }

    private class GetFileContent extends AsyncTask<String, Void, String> {
        //ProgressDialog progressDialog=null;
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            //progressDialog = Utils.getProcessDialog(EditActivity.this);
            //progressDialog.setMessage("读取文件中．．．");
          //  progressDialog.show();
        }

        @Override
        protected String doInBackground(String... params) {
            String path = params[0];
            return Utils.readFile(new File(path));

        }

        @Override
        protected void onPostExecute(String s) {
            super.onPostExecute(s);
           // progressDialog.dismiss();
            et_config.setText(s);
        }
    }
    private class SaveFileContent extends AsyncTask<String, Void, Void> {
        ProgressDialog progressDialog=null;
        String content;
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
           /* progressDialog = Utils.getProcessDialog(EditActivity.this);
            progressDialog.setMessage("保存文件中．．．");
            progressDialog.show();*/
            content=et_config.getText().toString();
        }

        @Override
        protected Void doInBackground(String... params) {
            String path = params[0];
            Utils.writeFile(new File(path),content);
            return null;
        }

        @Override
        protected void onPostExecute(Void s) {
            super.onPostExecute(s);
           // progressDiaLogTools.dismiss();
            Toast.makeText(EditActivity.this, "已保存", Toast.LENGTH_SHORT).show();
        }
    }
    private void setupActionBar() {
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setTitle("编辑");

            // Show the Up button in the action bar.
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.edit,menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            finish();
            return true;
        } else if (id == R.id.menu_save) {
            new SaveFileContent().execute(filename);
        }

        return super.onOptionsItemSelected(item);
    }
}
