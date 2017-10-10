package com.raylib.game_sample;

public class NativeLoader extends android.app.NativeActivity 
{
    static 
    {
       System.loadLibrary("openal");
       System.loadLibrary("raylib");
       System.loadLibrary("raylib_game");
    }
 }