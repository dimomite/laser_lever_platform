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
class PlatformStatusReader @Inject constructor(
    private val platformControl: PlatformControl,
) {
    fun status(): Flowable<PlatformStatus> = flow

    private val flow: Flowable<PlatformStatus> = Flowable.interval(
        1_000, TimeUnit.MILLISECONDS
    ).switchMap { readStatus() }
        .subscribeOn(Schedulers.io())
        .unsubscribeOn(Schedulers.io())
        .observeOn(AndroidSchedulers.mainThread())
        .replay(1).refCount()

    private fun stubStatus(idLong: Long): Flowable<PlatformStatus> {
        val id = idLong.toInt()
        val status = PlatformStatus(
            linear = LinearMovementState.values()[id % LinearMovementState.values().size],
            rotation = RotationMovementState.values()[id % RotationMovementState.values().size],
            error = PlatformError.values()[id % PlatformError.values().size],
        )

        return Flowable.just(status)
    }

    private fun readStatus(): Flowable<PlatformStatus> {
        val response = platformControl.readStatus().execute()
        return if (response.isSuccessful) {
            val ps = response.body()
            if (ps != null) {
                Flowable.just(ps)
            } else {
                Flowable.empty()
            }
        } else {
            Flowable.empty()
        }
    }

}
