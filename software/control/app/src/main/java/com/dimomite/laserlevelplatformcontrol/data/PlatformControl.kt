package com.dimomite.laserlevelplatformcontrol.data

import com.dimomite.laserlevelplatformcontrol.domain.PlatformStatus
import okhttp3.ResponseBody
import retrofit2.Call
import retrofit2.http.GET

interface PlatformControl {
    @GET("api/status")
    fun readStatus(): Call<PlatformStatus>

    @GET("favicon.ico")
    fun readFavIcon(): Call<ResponseBody>
}