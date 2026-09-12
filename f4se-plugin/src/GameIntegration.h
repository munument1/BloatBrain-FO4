#pragma once

namespace BloatBrain::GameIntegration
{
    // Called from F4SE's game-data-ready message on the game thread.
    void InitializeGameData();

    // May be called from the network worker. The actual TESGlobal write is
    // queued to F4SE's game thread.
    void QueueBridgeOnline(bool online);
}
