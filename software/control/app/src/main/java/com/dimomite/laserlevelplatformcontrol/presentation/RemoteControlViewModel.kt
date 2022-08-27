package com.dimomite.laserlevelplatformcontrol.presentation

import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import androidx.lifecycle.LiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.toLiveData
import com.dimomite.laserlevelplatformcontrol.data.PlatformStatusReader
import com.dimomite.laserlevelplatformcontrol.domain.LinearMovementState
import com.dimomite.laserlevelplatformcontrol.domain.PlatformError
import com.dimomite.laserlevelplatformcontrol.domain.PlatformStatus
import com.dimomite.laserlevelplatformcontrol.domain.RotationMovementState
import dagger.hilt.android.lifecycle.HiltViewModel
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

}
