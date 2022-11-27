package com.dimomite.laserlevelplatformcontrol.data

import com.dimomite.laserlevelplatformcontrol.domain.PlatformStatus
import okhttp3.ResponseBody
import retrofit2.Call
import retrofit2.http.GET
import retrofit2.http.POST
import retrofit2.http.Query

interface PlatformControl {
    @GET("api/status")
    fun readStatus(): Call<PlatformStatus>

    @GET("favicon.ico")
    fun readFavIcon(): Call<ResponseBody>

    @POST("api/move")
    fun move(@Query("dir") direction: String, @Query("dist") distance: Int): Call<ResponseBody>

    @POST("api/stopmove")
    fun stopMove(): Call<ResponseBody>

    @POST("api/turn")
    fun turn(@Query("dir") direction: String, @Query("dist") distance: Int): Call<ResponseBody>

    @POST("api/stopturn")
    fun stopTurn(): Call<ResponseBody>
}