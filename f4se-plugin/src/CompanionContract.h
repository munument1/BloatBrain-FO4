#pragma once

#include <string_view>

namespace BloatBrain::CompanionContract
{
    inline constexpr std::string_view kCompanionRefEditorID = "BB_FlyCompanionREF";
    inline constexpr std::string_view kCompanionBaseEditorID = "BB_FlyCompanion";
    inline constexpr std::string_view kNeuralKeywordEditorID = "BB_NeuralControlled";
    inline constexpr std::string_view kRecruitedGlobalEditorID = "BB_Recruited";
    inline constexpr std::string_view kBridgeOnlineGlobalEditorID = "BB_BridgeOnline";
    inline constexpr std::string_view kHomeMarkerEditorID = "BB_RedRocketHomeMarker";
    inline constexpr std::string_view kQuestEditorID = "BB_CompanionQuest";

    // The plugin must control only this persistent placed reference. Never select
    // actors merely because they use the Bloatfly race.
}
