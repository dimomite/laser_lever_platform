package com.dimomite.laserlevelplatformcontrol.data

import com.dimomite.laserlevelplatformcontrol.domain.LinearMovementState
import com.dimomite.laserlevelplatformcontrol.domain.PlatformError
import com.dimomite.laserlevelplatformcontrol.domain.PlatformStatus
import com.dimomite.laserlevelplatformcontrol.domain.RotationMovementState
import io.reactivex.rxjava3.android.schedulers.AndroidSchedulers
import io.reactivex.rxjava3.core.Flowable
import io.reactivex.rxjava3.schedulers.Schedulers
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class PlatformStatusReader @Inject constructor() {
    fun status(): Flowable<PlatformStatus> = flow

    private val flow: Flowable<PlatformStatus> = Flowable.interval(
        1_000, TimeUnit.MILLISECONDS
    ).switchMap {
        val id = it.toInt()
        val status = PlatformStatus(
            linear = LinearMovementState.values()[id % LinearMovementState.values().size],
            rotation = RotationMovementState.values()[id % RotationMovementState.values().size],
            error = PlatformError.values()[id % PlatformError.values().size],
        )
        Flowable.just(status)
    }
        .subscribeOn(Schedulers.io())
        .unsubscribeOn(Schedulers.io())
        .observeOn(AndroidSchedulers.mainThread())
        .replay(1).refCount()
}