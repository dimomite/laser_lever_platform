package com.dimomite.laserlevelplatformcontrol.domain

enum class LinearMovementState {
    Undefined,
    Stopped,
    MovingLeft,
    MovingRight,
    ReachedMaxLeft,
    ReachedMaxRight,
}
