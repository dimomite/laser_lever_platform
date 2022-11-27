package com.dimomite.laserlevelplatformcontrol.data

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.dimomite.laserlevelplatformcontrol.domain.*
import com.squareup.moshi.Moshi
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
    private val moshi = Moshi.Builder().build()
    private val directionAdapter = moshi.adapter(MoveDirection::class.java)
    private val turnAdapter = moshi.adapter(TurnDirection::class.java)

    private val okFlow: Flowable<Boolean> = Flowable.just(true)
    private val nokFlow: Flowable<Boolean> = Flowable.just(false)

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

    fun favicon(): Bitmap? {
        val response = platformControl.readFavIcon().execute()
        return if (response.isSuccessful) {
            BitmapFactory.decodeStream(response.body()!!.byteStream())
        } else null
    }

    fun move(dir: MoveDirection, distance: Int): Flowable<Boolean> {
        if (distance <= 0) return Flowable.just(false)

        return Flowable.fromCallable {
            platformControl.move(
                direction = directionAdapter.toJson(dir),
                distance = distance,
            ).execute().isSuccessful
        }
            .subscribeOn(Schedulers.io())
            .unsubscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
    }

    fun turn(dir: TurnDirection, distance: Int): Flowable<Boolean> {
        if (distance <= 0) return Flowable.just(false)

        return Flowable.fromCallable {
            platformControl.turn(
                direction = turnAdapter.toJson(dir),
                distance = distance,
            ).execute().isSuccessful
        }
            .subscribeOn(Schedulers.io())
            .unsubscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
    }

}
