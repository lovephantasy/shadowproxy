package android.shadowproxy.ListViewEx;

import android.content.Context;
import android.shadowproxy.R;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by lovefantasy on 17-2-22.
 */

public class ConfigAdapter extends BaseAdapter {
    private Context mContext = null;
    private LayoutInflater mLayoutInflater = null;
    private List<ConfigItem> mDatas = new ArrayList<>();
    private OnSelecListener mSelectListener = null;
    private OnDelListener mOnDelListener = null;
    private int mLastSel = -1;

    public ConfigAdapter(Context context) {
        mContext = context;
        mLayoutInflater = LayoutInflater.from(context);
    }

    public void addAll(List<ConfigItem> datas) {
        if (datas == null) return;
        mDatas.addAll(datas);
        notifyDataSetChanged();
    }
    public void add(ConfigItem data) {
        if (data == null) return;
        mDatas.add(data);
        notifyDataSetChanged();
    }
    public void remove(int  positon) {
        if (positon == AdapterView.INVALID_POSITION) return;
        mDatas.remove(positon);
        notifyDataSetChanged();
    }

    public void removeAll() {
        mDatas.clear();
        notifyDataSetChanged();
    }

    @Override
    public int getCount() {
        if (mDatas != null)
            return mDatas.size();
        else
            return -1;
    }

    public List<ConfigItem> getDatas() {
        return mDatas;
    }

    @Override

    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        ConfigHolder holder = null;
        if (convertView == null) {
            convertView = mLayoutInflater.inflate(R.layout.item_config, null);
            holder = new ConfigHolder(convertView);
            convertView.setTag(holder);
        } else {
            holder = (ConfigHolder) convertView.getTag();
        }
        holder.tv_del.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mOnDelListener != null) {
                    mOnDelListener.del(position);
                }
            }
        });
        holder.tv_name.setText(mDatas.get(position).mFilename);
        if (mDatas.get(position).mIsSelected) {
            mLastSel = position;
            holder.bt_select.setEnabled(false);
        }else {
            holder.bt_select.setEnabled(true);
        }

        holder.bt_select.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mSelectListener != null) {
                    if (mLastSel != -1)
                        mDatas.get(mLastSel).mIsSelected = false;
                    mDatas.get(position).mIsSelected = true;

                    mSelectListener.select(position,mLastSel);

                    mLastSel = position;
                }
            }
        });
        return convertView;
    }

    public void setOnSelectListener(OnSelecListener onSelectListener) {
        this.mSelectListener = onSelectListener;
    }
    public void setOnDelListener(OnDelListener onDelListener) {
        this.mOnDelListener = onDelListener;
    }
    public interface OnSelecListener {
        public void select(int position, int lastsel);
    }

    public interface OnDelListener{
        public void del(int position);
    }
}
