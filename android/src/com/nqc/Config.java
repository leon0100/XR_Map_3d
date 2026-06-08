package com.nqc;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.os.Build;


public class Config {

    private static final String PREF_NAME = "AppSettings";
    private static final String TAG = "Config";

    private static final String KEY_MAP_SOURCE_TYPE   = "mapSourceType";
    private static final String KEY_CURRENT_LEVEL     = "currentLevel";
    private static final String KEY_CURRENT_LON       = "currentLon";
    private static final String KEY_CURRENT_LATI      = "currentLati";
    private static final String KEY_EXIST_GOOGLE      = "existGoogle";
    private static final String KEY_CURRENT_LANGUAGE  = "currentLanguage";
    private static final String KEY_IN_CHINA          = "inChina";
    private static final String KEY_CRC_VALUE         = "crcValue";
    private static final String KEY_IS_FIRST_RUN      = "isFirstRun";

    public static void saveSoftwareParameters(Context context, int mapSourceType, int currentLevel, double currentLon,
                    double currentLati, int existGoogle, int currentLanguage, boolean inChina, int crcValue, boolean isFirstRun)
    {
        try {
            SharedPreferences prefs = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
            SharedPreferences.Editor editor = prefs.edit();

            editor.putInt(KEY_MAP_SOURCE_TYPE, mapSourceType);
            editor.putInt(KEY_CURRENT_LEVEL, currentLevel);
            editor.putFloat(KEY_CURRENT_LON, (float)currentLon);
            editor.putFloat(KEY_CURRENT_LATI, (float)currentLati);
            editor.putInt(KEY_EXIST_GOOGLE, existGoogle);
            editor.putInt(KEY_CURRENT_LANGUAGE, currentLanguage);
            editor.putBoolean(KEY_IN_CHINA, inChina);
            editor.putInt(KEY_CRC_VALUE, crcValue);
            editor.putBoolean(KEY_IS_FIRST_RUN, isFirstRun);

            editor.apply();
            Log.i(TAG, "SoftwareParameters saved.");
        } catch (Exception e) {
            Log.e(TAG, "saveSoftwareParameters error: " + e.getMessage());
        }
    }

    public static float[] loadSoftwareParameters(Context context)
    {
        float[] values = new float[9];

        try {
             SharedPreferences prefs = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
            values[0] = prefs.contains(KEY_MAP_SOURCE_TYPE) ? prefs.getInt(KEY_MAP_SOURCE_TYPE, 0) : 0;
            values[1] = prefs.contains(KEY_CURRENT_LEVEL) ? prefs.getInt(KEY_CURRENT_LEVEL, 0) : 0;
            values[2] = prefs.contains(KEY_CURRENT_LON) ? prefs.getFloat(KEY_CURRENT_LON, 0f) : 0f;
            values[3] = prefs.contains(KEY_CURRENT_LATI) ? prefs.getFloat(KEY_CURRENT_LATI, 0f) : 0f;
            values[4] = prefs.contains(KEY_EXIST_GOOGLE) ? prefs.getInt(KEY_EXIST_GOOGLE, 0) : 0;
            values[5] = prefs.contains(KEY_CURRENT_LANGUAGE) ? prefs.getInt(KEY_CURRENT_LANGUAGE, 0) : 0;
            values[6] = prefs.contains(KEY_IN_CHINA) ? (prefs.getBoolean(KEY_IN_CHINA, false) ? 1f : 0f) : 0f;
            values[7] = prefs.contains(KEY_CRC_VALUE) ? prefs.getInt(KEY_CRC_VALUE, 0) : 0;
            if (!prefs.contains(KEY_IS_FIRST_RUN)) {
                values[8] = 1f;
            } else {
                values[8] = prefs.getBoolean(KEY_IS_FIRST_RUN, false) ? 1f : 0f;
            }

        } catch (Exception e) {
            Log.e(TAG, "loadSoftwareParameters error: " + e.getMessage());
        }

        return values;
    }

    public static void clear(Context context) {
        SharedPreferences prefs = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
        prefs.edit().clear().apply();
        Log.i(TAG, "All settings cleared.");
    }



    public static int updateBatteryLevel(Context context)
    {
        try {
            IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
            Intent batteryStatus = context.registerReceiver(null, ifilter);

            if (batteryStatus != null) {
                int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
                int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
                if (level >= 0 && scale > 0) {
                    return (int)((level * 100.0f) / scale);
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "updateBatteryLevel error: " + e.getMessage());
        }

        return -1;
    }


    public static int updateNetworkStatus(Context context)
    {
        try {
            ConnectivityManager cm =
                    (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);

            if (cm == null)
                return 0;

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {

                android.net.Network network = cm.getActiveNetwork();
                if (network == null)
                    return 0;

                NetworkCapabilities capabilities = cm.getNetworkCapabilities(network);

                if (capabilities == null)
                    return 0;

                if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI))
                    return 4;

                if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR))
                    return 3;

                if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET))
                    return 4;

            }
            else {
                android.net.NetworkInfo info = cm.getActiveNetworkInfo();

                if (info != null && info.isConnected()) {

                    switch (info.getType()) {
                    case ConnectivityManager.TYPE_WIFI:
                        return 4;

                    case ConnectivityManager.TYPE_MOBILE:
                        return 3;

                    default:
                        return 1;
                    }
                }
            }

        } catch (Exception e) {
            Log.e(TAG, "updateNetworkStatus error: " + e.getMessage());
        }

        return 0;
    }



}
