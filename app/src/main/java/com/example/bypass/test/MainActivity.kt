package com.example.bypass.test

import android.os.Bundle
import android.widget.EditText
import androidx.appcompat.app.AppCompatActivity
import com.wind.utf8fixer.Utf8Fixer

class MainActivity : AppCompatActivity() {

    private val TAG = "MainActivity"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val text = findViewById<EditText>(R.id.editText)
        text.setText("\uD83D\uDE09")  // this is an emoji

        Utf8Fixer.init()  //  不调用这个一行的话，在android5.0和5.1机器上，testUtf8Crash必定crash
        testUtf8Crash(text.text.toString())
    }
    companion object {
        init {
            System.loadLibrary("test-lib")
        }

        @JvmStatic
        external fun testUtf8Crash(str: String)
    }
}