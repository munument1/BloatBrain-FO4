#include "pch.h"
#include "BridgeClient.h"

namespace
{
    constexpr std::string_view kBridgeHost = "127.0.0.1";
    constexpr std::uint16_t kBridgePort = 8765;
    constexpr auto kReconnectDelay = std::chrono::seconds(2);

    std::jthread g_bridgeThread;

    void BridgeWorker(std::stop_token stopToken)
    {
        BloatBrain::BridgeClient client;

        while (!stopToken.stop_requested()) {
            if (!client.IsConnected()) {
                if (!client.Connect(kBridgeHost, kBridgePort)) {
                    std::this_thread::sleep_for(kReconnectDelay);
                    continue;
                }
            }

            // Actor observation sampling and command dispatch will be added here.
            // Keep all blocking socket I/O off the Fallout 4 game thread.
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        client.Disconnect();
    }
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
    F4SE::Init(a_f4se);

    REX::INFO("BloatBrain-FO4 loading");
    g_bridgeThread = std::jthread(BridgeWorker);
    REX::INFO("BloatBrain bridge worker started ({}:{})", kBridgeHost, kBridgePort);

    return true;
}
