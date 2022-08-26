package com.dimomite.laserlevelplatformcontrol.domain

import com.squareup.moshi.Json

enum class LinearMovementState {
    @Json(name = "undef")
    Undefined,

    @Json(name = "stopped")
    Stopped,

    @Json(name = "movingleft")
    MovingLeft,

    @Json(name = "movingright")
    MovingRight,

    @Json(name = "maxleft")
    ReachedMaxLeft,

    @Json(name = "maxright")
    ReachedMaxRight,
}
