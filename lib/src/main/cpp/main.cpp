//
// Created by windysha on 2021/11/19.
//

#include <jni.h>
#include <string>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include "logger.h"
#include "func_replace.h"
#include "utf.h"
#include "elf_util.h"

static int api_level = -1;
static inline int get_android_api_level() {
    if (api_level < 0) {
        char api[PROP_VALUE_MAX];
        __system_property_get("ro.build.version.sdk", api);
        api_level = atoi(api);
    }
    return api_level;
}

static void replaced_Check(void *scope, bool entry, const char* fmt0, ...) {
    return;
}

void startFixUtf8Error(JNIEnv *env) {
    void *libart_handler = dlopen("libart.so", RTLD_NOW);
    LOGD(" startFixUtf8Error, lib art  : = [%p]", libart_handler);
    if (libart_handler == nullptr) {
        return;
    }

#ifdef __LP64__
    const char *convertUtf16MethodName = "_ZN3art26ConvertUtf16ToModifiedUtf8EPcPKtm";
#else
    const char *convertUtf16MethodName = "_ZN3art26ConvertUtf16ToModifiedUtf8EPcPKtj";
#endif
    void *convertUtf16MethodName_addr = dlsym(libart_handler, convertUtf16MethodName);
    LOGD(" startFixUtf8Error, ConvertUtf16ToModifiedUtf8 = [%p]", convertUtf16MethodName_addr);
    if (convertUtf16MethodName_addr == nullptr) {
        return;
    }
    DoFuncReplacing(convertUtf16MethodName_addr, (void *) BDFixer::ConvertUtf16ToModifiedUtf8);

#ifdef __LP64__
    const char *count_method_sym = "_ZN3art14CountUtf8BytesEPKtm";
#else
    const char* count_method_sym = "_ZN3art14CountUtf8BytesEPKtj";
#endif

    void *CountUtf8Bytes_addr = dlsym(libart_handler, count_method_sym);
    LOGD(" startFixUtf8Error, count_method_addr = [%p]", CountUtf8Bytes_addr);
    if (CountUtf8Bytes_addr == nullptr) {
        return;
    }
    DoFuncReplacing(CountUtf8Bytes_addr, (void *) BDFixer::CountUtf8Bytes);

    const char *convertUtf8MethodName = "_ZN3art26ConvertModifiedUtf8ToUtf16EPtPKc";
    void *convertUtf8Method_addr = dlsym(libart_handler, convertUtf8MethodName);
    LOGD(" startFixUtf8Error, ConvertModifiedUtf8ToUtf16 address = [%p]", convertUtf8Method_addr);
    if (convertUtf8Method_addr == nullptr) {
        return;
    }
    DoFuncReplacing(convertUtf8Method_addr, (void *) BDFixer::ConvertModifiedUtf8ToUtf16);

    void *CountModifiedUtf8Chars_addr =
        dlsym(libart_handler, "_ZN3art22CountModifiedUtf8CharsEPKc");  // oppo   char* const
    if (CountModifiedUtf8Chars_addr == nullptr) {
        CountModifiedUtf8Chars_addr =
            dlsym(libart_handler, "_ZN3art22CountModifiedUtf8CharsEPKcj");  // vivo const char*
    }
    LOGD(" startFixUtf8Error, CountModifiedUtf8Chars address = [%p]", CountModifiedUtf8Chars_addr);
    if (CountModifiedUtf8Chars_addr == nullptr) {
        return;
    }
    DoFuncReplacing(CountModifiedUtf8Chars_addr, (void *) BDFixer::CountModifiedUtf8Chars);


//   void JNIEnvExt::SetCheckJniEnabled(bool enabled)
    const char *SetCheckJniEnabled_symbol = "_ZN3art9JNIEnvExt18SetCheckJniEnabledEb";
    void *SetCheckJniEnabled_address = dlsym(libart_handler, SetCheckJniEnabled_symbol);
    LOGD(" startFixUtf8Error, SetCheckJniEnabled address = [%p]", SetCheckJniEnabled_address);
    if (SetCheckJniEnabled_address != nullptr) {
        auto SetCheckJniEnabled_func =
            reinterpret_cast<void (*)(void *, bool)>(SetCheckJniEnabled_address);
        SetCheckJniEnabled_func(env, false);
    }

    const char *Scope_Check_symbol_1 = "_ZN3art11ScopedCheck5CheckEbPKcz.constprop.161";
    const char *Scope_Check_symbol_2 = "_ZN3art11ScopedCheck5CheckEbPKcz.constprop.128";
    const char *Scope_Check_symbol_3 = "_ZN3art11ScopedCheck5CheckEbPKcz.constprop.129";
    const char *Scope_Check_symbol_4 = "_ZN3art11ScopedCheck5CheckEbPKcz.constprop.130";
    const char *Scope_Check_symbol_5 = "_ZN3art11ScopedCheck5CheckEbPKcz.constprop.131";
    const char *Scope_Check_symbol_6 =
        "_ZN3art11ScopedCheck5CheckEbPKcz.constprop.132"; // guessed, may not exist
    const char *Scope_Check_symbol_7 =
        "_ZN3art11ScopedCheck5CheckEbPKcz.constprop.127"; // guessed, may not exist

    auto elf_img = std::make_unique<BDFixer::ElfImg>("system/lib/libart.so");
    const char *symble_list[] =
        {Scope_Check_symbol_1, Scope_Check_symbol_2, Scope_Check_symbol_3, Scope_Check_symbol_4,
         Scope_Check_symbol_5, Scope_Check_symbol_6, Scope_Check_symbol_7};
    for (auto sym : symble_list) {
        auto address = (void *) elf_img->getSymbAddress(sym);
        if (address != nullptr) {
            LOGD(" JNI_OnLoad start, scoped check symble = %s  address = %p", sym, address);
            DoFuncReplacing(address, (void *) replaced_Check);
        }
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGD("This jni version is not supported");
        return JNI_VERSION_1_6;
    }

    int device_api_level = get_android_api_level();
    if (device_api_level == 21 || device_api_level == 22) {
        startFixUtf8Error(env);
    }
    return JNI_VERSION_1_6;
}