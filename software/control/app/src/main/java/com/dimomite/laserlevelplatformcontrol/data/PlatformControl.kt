package com.dimomite.laserlevelplatformcontrol.data

import com.dimomite.laserlevelplatformcontrol.domain.PlatformStatus
import retrofit2.Call
import retrofit2.http.GET

interface PlatformControl {
    @GET("status")
    fun readStatus(): Call<PlatformStatus>
}