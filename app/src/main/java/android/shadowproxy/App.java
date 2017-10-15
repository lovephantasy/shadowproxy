package android.shadowproxy;

import android.app.Application;
import android.content.Context;


/**
 * Created by lovefantasy on 17-3-3.
 */

public class App extends Application {
    private static Context context;

    @Override
    public void onCreate() {
        super.onCreate();
        context = getApplicationContext();

    }

    static public Context getContext() {
        return context;
    }

}
