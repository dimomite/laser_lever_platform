package com.dimomite.laserlevelplatformcontrol.domain

import com.squareup.moshi.Json

enum class RotationMovementState {
    @Json(name = "undef")
    Undefined,

    @Json(name = "stopped")
    Stopped,

    @Json(name = "cw")
    TurningCW,

    @Json(name = "ccw")
    TurningCCW,
}
