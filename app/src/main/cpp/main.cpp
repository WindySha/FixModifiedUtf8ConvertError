
#include <jni.h>
#include <memory>
#include "main.h"
#include "json.hpp"

extern "C"
JNIEXPORT void JNICALL
Java_com_example_bypass_test_MainActivity_testUtf8Crash(JNIEnv *env, jclass clazz, jstring str) {
    LOGD("   testUtf68Convert start !!");
    jboolean isCopy;
    auto testStr = env->GetStringUTFChars(str, &isCopy);

    std::string param("{");
    param.append("\"test\":");
    param.append("\"");
    param.append(testStr);
    param.append("\"");
    param.append("}");

    auto js = nlohmann::json::parse(param);

    jstring result = env->NewStringUTF(testStr);
    LOGD("   testUtf16Convert end !!  -> [%s]", testStr);
    env->ReleaseStringUTFChars(str, testStr);
}