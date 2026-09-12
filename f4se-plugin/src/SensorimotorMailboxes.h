#pragma once

#include "LatestMailbox.h"
#include "ProtocolV2.h"

namespace BloatBrain
{
    using SensoryFrameMailbox = LatestMailbox<ProtocolV2::SensoryFrame>;
    using MotorCommandMailbox = LatestMailbox<ProtocolV2::MotorCommand>;

    // Owned by the future game-thread integration boundary. The game thread
    // publishes sensory snapshots and consumes motor snapshots; the socket
    // worker does the inverse without ever touching Fallout objects.
    struct SensorimotorMailboxes
    {
        SensoryFrameMailbox sensory_frames;
        MotorCommandMailbox motor_commands;
    };
}
