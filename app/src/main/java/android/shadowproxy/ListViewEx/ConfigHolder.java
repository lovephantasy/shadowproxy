package android.shadowproxy.ListViewEx;

import android.shadowproxy.R;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

/**
 * Created by lovefantasy on 2017/5/20.
 */

public class ConfigHolder {
    public View view=null;
    public TextView tv_name=null;
    public  Button bt_select=null;
    public TextView tv_del=null;
    public ConfigHolder(View itemView) {
        view=itemView;
        tv_name= (TextView) view.findViewById(R.id.tv_name);
        bt_select = (Button) view.findViewById(R.id.bt_select);
        tv_del = (TextView) view.findViewById(R.id.tv_del);
    }
}