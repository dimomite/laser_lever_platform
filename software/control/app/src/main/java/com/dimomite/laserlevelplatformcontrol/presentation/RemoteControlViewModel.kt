package com.dimomite.laserlevelplatformcontrol.presentation

import android.widget.RadioGroup
import androidx.lifecycle.LiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.toLiveData
import com.dimomite.laserlevelplatformcontrol.R
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

    fun moveLeft(rg: RadioGroup) {
        subs.add(statusReader.move(MoveDirection.Left, getDuration(rg))
            .subscribe { Timber.d("moveLeft() finished with status: $it") }
        )
    }

    fun moveRight(rg: RadioGroup) {
        subs.add(statusReader.move(MoveDirection.Right, getDuration(rg))
            .subscribe { Timber.d("moveRight() finished with status: $it") }
        )
    }

    fun stopMove() {
        subs.add(statusReader.stopMove()
            .subscribe { Timber.d("stopMove() finished with status: $it") })
    }

    fun turnCW(rg: RadioGroup) {
        subs.add(statusReader.turn(TurnDirection.CW, getDuration(rg))
            .subscribe { Timber.d("turnCW() finished with status: $it") }
        )
    }

    fun turnCCW(rg: RadioGroup) {
        subs.add(statusReader.turn(TurnDirection.CCW, getDuration(rg))
            .subscribe { Timber.d("turnCCW() finished with status: $it") }
        )
    }

    fun stopTurn() {
        subs.add(statusReader.stopTurn()
            .subscribe { Timber.d("stopTurn() finished with status: $it") })
    }

    private fun getDuration(rg: RadioGroup): Int =
        when (val id = rg.checkedRadioButtonId) {
            R.id.rb_10 -> 10
            R.id.rb_100 -> 100
            R.id.rb_500 -> 500
            else -> {
                Timber.d("No mapping for view id: $id")
                0
            }
        }

}
