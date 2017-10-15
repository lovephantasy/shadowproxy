package android.shadowproxy.ListViewEx;

/**
 * Created by lovefantasy on 2017/5/20.
 */

public class ConfigItem {
   public String mFilename;
   public Boolean mIsSelected=false;

    public ConfigItem(String filename, boolean isSelected) {
        mFilename=filename;
        mIsSelected=isSelected;
    }
}
