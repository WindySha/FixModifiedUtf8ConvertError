package com.wind.utf8fixer;

import android.os.Build;

public class Utf8Fixer {
    public static void init() {
        // only fix error on Android 5.0 and 5.1
        if (Build.VERSION.SDK_INT == Build.VERSION_CODES.LOLLIPOP
                || Build.VERSION.SDK_INT == Build.VERSION_CODES.LOLLIPOP_MR1) {
            System.loadLibrary("utf8_fixer");
        }
    }
}
