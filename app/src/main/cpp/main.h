
#pragma once

#include <android/log.h>

#define LOG_TAG  "Test"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,  LOG_TAG, __VA_ARGS__)

