# FixModifiedUtf8ConvertError
This is a library that can fix the crash on android 5.0 and 5.1 caused by modified utf8 converting.

# What's this
On Android 5.0 and 5.1, using `env->NewStringUTF` or `env->GetStringUTFChars()` may cause crashing, especially when the string contains emoij.
This is a bug on android 5.

The issue detail is : https://issuetracker.google.com/issues/37013126

And Google have fix this issue on android 6, the commits are:
1. [Be more lenient with 4 byte UTF-8 sequences.](https://android-review.googlesource.com/c/platform/art/+/130121)
2. [Emit 4 byte UTF-sequences in place of encoded surrogate pairs.](https://android-review.googlesource.com/c/platform/art/+/132241)

This library can fix this kind of crash on android 5.0.

# How to use
Build an aar from lib module, import the aar to your android project, then in your main application file, add this:

```
import com.wind.utf8fixer.Utf8Fixer

protected void attachBaseContext(Context base) {
    Utf8Fixer.init();
    super.attachBaseContext(base);
}
```
