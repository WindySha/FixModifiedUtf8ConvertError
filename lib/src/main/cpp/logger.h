//
// Created by windysha on 2021/11/19.
//

#ifndef UTF8CONVERTERRORFIXER_LIB_SRC_MAIN_CPP_LOGGER_H
#define UTF8CONVERTERRORFIXER_LIB_SRC_MAIN_CPP_LOGGER_H

#include <android/log.h>

#define LOG_TAG "Utf8_Fixer"
//#define DEBUG 1

#ifdef DEBUG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...)
#define LOGE(...)
#endif

#endif //UTF8CONVERTERRORFIXER_LIB_SRC_MAIN_CPP_LOGGER_H
