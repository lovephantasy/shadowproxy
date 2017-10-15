package android.shadowproxy;

import android.app.ProgressDialog;
import android.content.Context;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;


/**
 * Created by lovefantasy on 2017/5/20.
 */

public class Utils {

    public static ProgressDialog getProcessDialog(Context context) {
        ProgressDialog progressDialog = null;
        progressDialog = new ProgressDialog(context);
        progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        progressDialog.setIndeterminate(false);
        progressDialog.setCancelable(false);
        return progressDialog;
    }

    public static void writeFile(File file, Object content) {
        try {
            FileOutputStream fileOutputStream=null;
            if (file.exists())
                 fileOutputStream = new FileOutputStream(file);
            else
                file.createNewFile();
            if (fileOutputStream==null)return;
            if (content instanceof String) {
                fileOutputStream.write(((String)content).getBytes());

            }else if (content instanceof byte[]){
                fileOutputStream.write((byte[]) content);
            }
            fileOutputStream.flush();
            fileOutputStream.close();
        }  catch (IOException e) {
            e.printStackTrace();
        }

    }

    public static String readFile(File file) {
       // LogTools.e("readfile",file.getPath());
        FileInputStream in = null;
        StringBuilder stringBuilder = new StringBuilder();
        char data[] = new char[512];
        int len;
        try {
            in = new FileInputStream(file);
            InputStreamReader inputStreamReader = new InputStreamReader(in);
            while ((len = inputStreamReader.read(data)) != -1) {
                stringBuilder.append(data, 0, len);
            }
            in.close();
            inputStreamReader.close();
        } catch (IOException e) {
            e.printStackTrace();
            return "read file error";
        }
        return stringBuilder.toString();
    }
}
