#pragma once

#include <cstdint>

void remoteContolServerTaks(void *args);

typedef enum
{
    Undefined,

    Left,
    Right,
    CW,
    CCW,
    StopMove,
    StopTurn,
} ActionCommandType;

typedef struct
{
    ActionCommandType type;
    int32_t duration;
} ActionCommand;
