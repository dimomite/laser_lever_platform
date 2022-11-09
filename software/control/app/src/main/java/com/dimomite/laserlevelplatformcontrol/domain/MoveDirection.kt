package com.dimomite.laserlevelplatformcontrol.domain

import com.squareup.moshi.Json

enum class MoveDirection {
    @Json(name = "left")
    Left,

    @Json(name = "right")
    Right,
}