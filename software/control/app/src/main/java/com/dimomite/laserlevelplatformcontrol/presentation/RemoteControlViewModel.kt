package com.dimomite.laserlevelplatformcontrol.presentation

import android.view.View
import androidx.lifecycle.LiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.toLiveData
import com.dimomite.laserlevelplatformcontrol.data.PlatformStatusReader
import com.dimomite.laserlevelplatformcontrol.domain.*
import dagger.hilt.android.lifecycle.HiltViewModel
import io.reactivex.rxjava3.disposables.CompositeDisposable
import timber.log.Timber
import javax.inject.Inject

@HiltViewModel
class RemoteControlViewModel @Inject constructor(
    private val statusReader: PlatformStatusReader,
) : ViewModel() {
//    val icon: Drawable?

//    init {
//        val favicon = statusReader.favicon()
//        icon = if (favicon != null) {
//            BitmapDrawable(favicon)
//        } else null
//    }

    private val subs = CompositeDisposable()

    override fun onCleared() {
        subs.clear()
        super.onCleared()
    }

    val data: LiveData<RemoteControlUiModel> = statusReader.status()
        .map { modelConversion(it) }
        .toLiveData()

    internal fun modelConversion(status: PlatformStatus): RemoteControlUiModel =
        RemoteControlUiModel(
            linearMovementState = linearMovementToText(status.linear),
            rotationMovementState = rotationMovementToText(status.rotation),
            error = errorToText(status.error),
        )

    internal fun linearMovementToText(lm: LinearMovementState): String = lm.toString()

    internal fun rotationMovementToText(rm: RotationMovementState): String = rm.toString()

    internal fun errorToText(er: PlatformError): String = er.toString()

    fun moveLeft(v: View) {
        subs.add(statusReader.move(MoveDirection.Left, 100)
            .subscribe { Timber.d("moveLeft() finished with status: $it") }
        )
    }

    fun moveRight(v: View) {
        subs.add(statusReader.move(MoveDirection.Right, 100)
            .subscribe { Timber.d("moveRight() finished with status: $it") }
        )
    }

    fun turnCW(v: View) {
        subs.add(statusReader.turn(TurnDirection.CW, 50)
            .subscribe { Timber.d("turnCW() finished with status: $it") }
        )
    }

    fun turnCCW(v: View) {
        subs.add(statusReader.turn(TurnDirection.CCW, 50)
            .subscribe { Timber.d("turnCCW() finished with status: $it") }
        )
    }

}
