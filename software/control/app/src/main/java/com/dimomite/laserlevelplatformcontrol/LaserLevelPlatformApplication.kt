package com.dimomite.laserlevelplatformcontrol

import android.app.Application
import dagger.hilt.android.HiltAndroidApp
import timber.log.Timber

@HiltAndroidApp
class LaserLevelPlatformApplication : Application() {
    companion object {
        private const val APP_TAG = "LLPC"
    }

    override fun onCreate() {
        super.onCreate()

        initLogging()
    }

    private fun initLogging() {
        if (Timber.forest().isEmpty()) {
            Timber.plant(object  : Timber.DebugTree() {
                override fun log(priority: Int, tag: String?, message: String, t: Throwable?) {
                    super.log(priority, APP_TAG, "$tag: $message", t)
                }
            })
        }
    }
}