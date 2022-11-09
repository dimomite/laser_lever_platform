package com.dimomite.laserlevelplatformcontrol.domain

import com.squareup.moshi.Json

enum class TurnDirection {
    @Json(name = "cw")
    CW,

    @Json(name = "ccw")
    CCW,
}