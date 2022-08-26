package com.dimomite.laserlevelplatformcontrol.domain

import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class PlatformStatus(
    @field:Json(name = "linear")
    val linear: LinearMovementState,

    @field:Json(name = "rotation")
    val rotation: RotationMovementState,

    @field:Json(name = "error")
    val error: PlatformError,
)
