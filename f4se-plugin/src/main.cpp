#include "pch.h"
#include "BridgeClient.h"
#include "CompanionContract.h"
#include "GameIntegration.h"

namespace
{
    constexpr std::string_view kBridgeHost = "127.0.0.1";
    constexpr std::uint16_t kBridgePort = 8765;
    constexpr auto kReconnectDelay = std::chrono::seconds(2);
    constexpr auto kSmokeTickDelay = std::chrono::seconds(1);

    std::jthread g_bridgeThread;

    void OnF4SEMessage(F4SE::MessagingInterface::Message* message)
    {
        if (message &&
            message->type == F4SE::MessagingInterface::kGameDataReady &&
            message->data != nullptr) {
            BloatBrain::GameIntegration::InitializeGameData();
        }
    }

    std::string MakeSmokeObservation(std::uint64_t tick)
    {
        return
            "{\"protocol_version\":1,\"type\":\"observation\",\"tick\":" +
            std::to_string(tick) +
            ",\"target_visible\":true,\"target_distance\":200.0,"
            "\"target_bearing_deg\":0.0,\"target_elevation_deg\":0.0,"
            "\"health_fraction\":1.0,\"recently_damaged\":false,"
            "\"in_combat\":true}";
    }

    void BridgeWorker(std::stop_token stopToken)
    {
        BloatBrain::BridgeClient client;
        std::uint64_t tick = 0;

        while (!stopToken.stop_requested()) {
            if (!client.IsConnected()) {
                if (!client.Connect(kBridgeHost, kBridgePort)) {
                    BloatBrain::GameIntegration::QueueBridgeOnline(false);
                    std::this_thread::sleep_for(kReconnectDelay);
                    continue;
                }
            }

            const auto observation = MakeSmokeObservation(tick++);
            if (!client.SendLine(observation)) {
                BloatBrain::GameIntegration::QueueBridgeOnline(false);
                std::this_thread::sleep_for(kReconnectDelay);
                continue;
            }

            const auto response = client.ReceiveLine();
            if (!response) {
                BloatBrain::GameIntegration::QueueBridgeOnline(false);
                std::this_thread::sleep_for(kReconnectDelay);
                continue;
            }

            BloatBrain::GameIntegration::QueueBridgeOnline(true);
            REX::DEBUG("BloatBrain smoke action: {}", *response);

            // Replace this synthetic observation loop with real sampling from the
            // single persistent actor named by CompanionContract::kCompanionRefEditorID.
            // Never scan/control every actor that happens to use the Bloatfly race.
            // All Fallout 4 object access must stay on the game thread; blocking
            // socket I/O intentionally stays on this worker thread.
            std::this_thread::sleep_for(kSmokeTickDelay);
        }

        client.Disconnect();
        BloatBrain::GameIntegration::QueueBridgeOnline(false);
    }
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
    F4SE::Init(a_f4se);

    REX::INFO("BloatBrain-FO4 loading");
    REX::INFO(
        "BloatBrain companion contract: {}",
        BloatBrain::CompanionContract::kCompanionRefEditorID);

    const auto* messaging = F4SE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnF4SEMessage)) {
        REX::CRITICAL("BloatBrain could not register the F4SE message listener");
        return false;
    }

    g_bridgeThread = std::jthread(BridgeWorker);
    REX::INFO("BloatBrain bridge worker started ({}:{})", kBridgeHost, kBridgePort);

    return true;
}
