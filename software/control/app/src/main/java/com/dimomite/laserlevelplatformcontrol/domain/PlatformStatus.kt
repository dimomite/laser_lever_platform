package com.dimomite.laserlevelplatformcontrol.domain

data class PlatformStatus(
    val linear: LinearMovementState,
    val rotation: RotationMovementState,
    val error: PlatformError,
)