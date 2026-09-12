#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace BloatBrain::ProtocolV2
{
    inline constexpr std::uint64_t kProtocolVersion = 2;

    struct SensoryFrame
    {
        std::uint64_t tick{};
        float dt{};
        float optic_flow_left{};
        float optic_flow_right{};
        float optic_flow_up{};
        float optic_flow_down{};
        float looming_left{};
        float looming_center{};
        float looming_right{};
        float familiar_cue_left{};
        float familiar_cue_center{};
        float familiar_cue_right{};
        float damage_signal{};
        float health_fraction{};

        friend bool operator==(const SensoryFrame&, const SensoryFrame&) = default;
    };

    struct MotorCommand
    {
        std::uint64_t tick{};
        float yaw{};
        float pitch{};
        float lift{};
        float thrust{};
        float attack_drive{};

        friend bool operator==(const MotorCommand&, const MotorCommand&) = default;
    };

    [[nodiscard]] bool Validate(const SensoryFrame& frame, std::string* error = nullptr);
    [[nodiscard]] bool Validate(const MotorCommand& command, std::string* error = nullptr);

    [[nodiscard]] std::optional<std::string> Serialize(
        const SensoryFrame& frame,
        std::string* error = nullptr);
    [[nodiscard]] std::optional<std::string> Serialize(
        const MotorCommand& command,
        std::string* error = nullptr);

    [[nodiscard]] std::optional<SensoryFrame> ParseSensoryFrame(
        std::string_view json,
        std::string* error = nullptr);
    [[nodiscard]] std::optional<MotorCommand> ParseMotorCommand(
        std::string_view json,
        std::string* error = nullptr);
}
