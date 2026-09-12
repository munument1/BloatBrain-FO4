#include "pch.h"
#include "CompanionContract.h"
#include "GameIntegration.h"

namespace BloatBrain::GameIntegration
{
    namespace
    {
        std::atomic_bool g_bridgeOnline{ false };

        template <class T>
        T* LookupByEditorID(std::string_view editorID)
        {
            const RE::BSFixedString fixedEditorID{ editorID.data() };
            return RE::TESForm::GetFormByEditorID<T>(fixedEditorID);
        }

        void SetBridgeOnlineOnGameThread(bool online)
        {
            auto* bridgeOnline = LookupByEditorID<RE::TESGlobal>(
                CompanionContract::kBridgeOnlineGlobalEditorID);
            if (!bridgeOnline) {
                REX::WARN(
                    "BloatBrain global not found: {}",
                    CompanionContract::kBridgeOnlineGlobalEditorID);
                return;
            }

            bridgeOnline->value = online ? 1.0F : 0.0F;
            REX::INFO("BloatBrain bridge state: {}", online ? "online" : "offline");
        }
    }

    void InitializeGameData()
    {
        auto* companion = LookupByEditorID<RE::Actor>(
            CompanionContract::kCompanionRefEditorID);
        if (!companion) {
            REX::WARN(
                "BloatBrain companion reference not found: {}. Is BloatBrainFO4.esp enabled?",
                CompanionContract::kCompanionRefEditorID);
        } else {
            REX::INFO(
                "BloatBrain companion acquired: {} (form {:08X})",
                CompanionContract::kCompanionRefEditorID,
                companion->GetFormID());
        }

        // Synchronize the ESP fallback state even when the worker connected
        // before the game's forms became available.
        SetBridgeOnlineOnGameThread(g_bridgeOnline.load(std::memory_order_acquire));
    }

    void QueueBridgeOnline(bool online)
    {
        const bool previous = g_bridgeOnline.exchange(online, std::memory_order_acq_rel);
        if (previous == online) {
            return;
        }

        const auto* tasks = F4SE::GetTaskInterface();
        if (!tasks) {
            REX::WARN("BloatBrain could not queue bridge state: F4SE task interface unavailable");
            return;
        }

        tasks->AddTask([online]() {
            SetBridgeOnlineOnGameThread(online);
        });
    }
}
