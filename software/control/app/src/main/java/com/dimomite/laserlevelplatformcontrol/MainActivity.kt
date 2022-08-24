package com.dimomite.laserlevelplatformcontrol

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.dimomite.laserlevelplatformcontrol.data.PlatformStatusReader
import dagger.hilt.android.AndroidEntryPoint
import io.reactivex.rxjava3.disposables.Disposable
import timber.log.Timber
import javax.inject.Inject

@AndroidEntryPoint
class MainActivity : AppCompatActivity() {
    @Inject
    lateinit var statusReader: PlatformStatusReader

    private var subscription: Disposable? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    override fun onResume() {
        super.onResume()

        if (subscription == null) {
            subscription = statusReader.status().subscribe {
                Timber.d("Status update: $it")
            }
        }
    }

    override fun onPause() {
        subscription?.dispose()
        subscription = null

        super.onPause()
    }

}