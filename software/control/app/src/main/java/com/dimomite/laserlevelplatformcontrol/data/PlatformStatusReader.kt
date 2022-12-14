package com.dimomite.laserlevelplatformcontrol.data

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.dimomite.laserlevelplatformcontrol.domain.*
import io.reactivex.rxjava3.android.schedulers.AndroidSchedulers
import io.reactivex.rxjava3.core.Flowable
import io.reactivex.rxjava3.schedulers.Schedulers
import java.io.IOException
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class PlatformStatusReader @Inject constructor(
    private val platformControl: PlatformControl,
) {
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
        val response = try {
            platformControl.readStatus().execute()
        } catch (_: IOException) {
            return Flowable.empty()
        }

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
        val response = try {
            platformControl.readFavIcon().execute()
        } catch (_: IOException) {
            return null
        }

        return if (response.isSuccessful) {
            BitmapFactory.decodeStream(response.body()!!.byteStream())
        } else null
    }

    fun move(dir: MoveDirection, distance: Int): Flowable<Boolean> {
        if (distance <= 0) return Flowable.just(false)

        return Flowable.fromCallable {
            val response = try {
                platformControl.move(
                    direction = dir.name.lowercase(),
                    distance = distance,
                ).execute()
            } catch (_: IOException) {
                return@fromCallable false
            }
            response.isSuccessful
        }
            .subscribeOn(Schedulers.io())
            .unsubscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
    }

    fun stopMove(): Flowable<Boolean> {
        return Flowable.fromCallable {
            val response = try {
                platformControl.stopMove().execute()
            } catch (_: IOException) {
                return@fromCallable false
            }
            response.isSuccessful
        }
            .subscribeOn(Schedulers.io())
            .unsubscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
    }

    fun turn(dir: TurnDirection, distance: Int): Flowable<Boolean> {
        if (distance <= 0) return Flowable.just(false)

        return Flowable.fromCallable {
            val response = try {
                platformControl.turn(
                    direction = dir.name.lowercase(),
                    distance = distance,
                ).execute()
            } catch (_: IOException) {
                return@fromCallable false
            }
            response.isSuccessful
        }
            .subscribeOn(Schedulers.io())
            .unsubscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
    }

    fun stopTurn(): Flowable<Boolean> {
        return Flowable.fromCallable {
            val response = try {
                platformControl.stopTurn().execute()
            } catch (_: IOException) {
                return@fromCallable false
            }
            response.isSuccessful
        }
            .subscribeOn(Schedulers.io())
            .unsubscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
    }

}
