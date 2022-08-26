package com.dimomite.laserlevelplatformcontrol.domain

import com.squareup.moshi.Json

enum class PlatformError {
    @Json(name = "none")
    None,

    @Json(name = "bothendstopsactive")
    BothEndStopsActive,
}
