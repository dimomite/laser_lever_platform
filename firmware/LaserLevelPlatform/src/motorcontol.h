#pragma once

enum class LinearMovementState
{
    UNDEFINED,
    STOPPED,
    MOVING_LEFT,
    MOVING_RIGHT,
    REACHED_MAX_LEFT,
    REACHED_MAX_RIGHT,
};

enum class RotationMovementState
{
    UNDEFINED,
    STOPPED,
    TURNING_CW,
    TURNING_CCW,
};

enum class PlatformError
{
    NONE,
    BOTH_END_STOPS_ACTIVE,
};

typedef struct
{
    LinearMovementState linear;
    RotationMovementState rotation;
    PlatformError error;
} PlatformStatus;
