#include "ProtocolV2.h"
#include "SensorimotorMailboxes.h"

#include <atomic>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <limits>
#include <string>
#include <thread>

namespace
{
    using BloatBrain::ProtocolV2::MotorCommand;
    using BloatBrain::ProtocolV2::SensoryFrame;

    SensoryFrame MakeFrame()
    {
        return SensoryFrame{
            .tick = 42, .dt = 0.05F,
            .optic_flow_left = 0.1F, .optic_flow_right = 0.2F,
            .optic_flow_up = 0.3F, .optic_flow_down = 0.4F,
            .looming_left = 0.5F, .looming_center = 0.6F, .looming_right = 0.7F,
            .familiar_cue_left = 0.8F, .familiar_cue_center = 0.9F,
            .familiar_cue_right = 1.0F, .damage_signal = 0.25F,
            .health_fraction = 0.75F
        };
    }

    void TestSensoryRoundTrip()
    {
        const auto source = MakeFrame();
        const auto json = BloatBrain::ProtocolV2::Serialize(source);
        assert(json);
        assert(BloatBrain::ProtocolV2::ParseSensoryFrame(*json) == source);
    }

    void TestMotorRoundTrip()
    {
        const MotorCommand source{
            .tick = 7, .yaw = -0.3F, .pitch = 0.2F, .lift = -0.5F,
            .thrust = 0.75F, .attack_drive = 0.1F
        };
        const auto json = BloatBrain::ProtocolV2::Serialize(source);
        assert(json);
        assert(BloatBrain::ProtocolV2::ParseMotorCommand(*json) == source);
    }

    void TestMalformedAndInvalidValuesAreRejected()
    {
        constexpr std::string_view negativeTick =
            R"({"protocol_version":2,"type":"motor_command","tick":-1,"yaw":0,"pitch":0,"lift":0,"thrust":0,"attack_drive":0})";
        constexpr std::string_view fractionalTick =
            R"({"protocol_version":2,"type":"motor_command","tick":1.5,"yaw":0,"pitch":0,"lift":0,"thrust":0,"attack_drive":0})";
        constexpr std::string_view outOfRange =
            R"({"protocol_version":2,"type":"motor_command","tick":1,"yaw":1.01,"pitch":0,"lift":0,"thrust":0,"attack_drive":0})";
        constexpr std::string_view duplicateTick =
            R"({"protocol_version":2,"type":"motor_command","tick":1,"tick":2,"yaw":0,"pitch":0,"lift":0,"thrust":0,"attack_drive":0})";
        assert(!BloatBrain::ProtocolV2::ParseMotorCommand(negativeTick));
        assert(!BloatBrain::ProtocolV2::ParseMotorCommand(fractionalTick));
        assert(!BloatBrain::ProtocolV2::ParseMotorCommand(outOfRange));
        assert(!BloatBrain::ProtocolV2::ParseMotorCommand(duplicateTick));

        auto frame = MakeFrame();
        frame.dt = std::numeric_limits<float>::infinity();
        assert(!BloatBrain::ProtocolV2::Serialize(frame));

        MotorCommand command{};
        command.yaw = std::numeric_limits<float>::quiet_NaN();
        assert(!BloatBrain::ProtocolV2::Serialize(command));
    }

    void TestPythonFieldNamesAreExact()
    {
        const auto json = BloatBrain::ProtocolV2::Serialize(MakeFrame());
        assert(json);
        for (const std::string_view name : {
                 "optic_flow_left", "optic_flow_right", "optic_flow_up", "optic_flow_down",
                 "looming_left", "looming_center", "looming_right",
                 "familiar_cue_left", "familiar_cue_center", "familiar_cue_right",
                 "damage_signal", "health_fraction" }) {
            assert(json->find(name) != std::string::npos);
        }
        assert(json->find("target_bearing") == std::string::npos);
        assert(json->find("player_") == std::string::npos);
    }

    void TestLatestMailboxRejectsStaleTicks()
    {
        BloatBrain::SensorimotorMailboxes mailboxes;
        auto& mailbox = mailboxes.motor_commands;
        assert(mailbox.Publish(MotorCommand{ .tick = 2 }));
        assert(!mailbox.Publish(MotorCommand{ .tick = 2 }));
        assert(!mailbox.Publish(MotorCommand{ .tick = 1 }));
        assert(mailbox.Snapshot()->tick == 2);
        assert(!mailbox.SnapshotNewerThan(2));
        mailbox.Clear();
        assert(!mailbox.Snapshot());
        assert(mailbox.Publish(MotorCommand{ .tick = 0 }));
    }

    void TestMailboxIsThreadSafe()
    {
        BloatBrain::SensorimotorMailboxes mailboxes;
        auto& mailbox = mailboxes.sensory_frames;
        std::atomic_bool start{ false };
        std::jthread lowWriter([&] {
            while (!start.load()) {}
            for (std::uint64_t tick = 0; tick < 1000; tick += 2) {
                auto frame = MakeFrame();
                frame.tick = tick;
                static_cast<void>(mailbox.Publish(frame));
            }
        });
        std::jthread highWriter([&] {
            while (!start.load()) {}
            for (std::uint64_t tick = 1; tick < 1000; tick += 2) {
                auto frame = MakeFrame();
                frame.tick = tick;
                static_cast<void>(mailbox.Publish(frame));
            }
        });
        start = true;
        lowWriter.join();
        highWriter.join();
        assert(mailbox.Snapshot()->tick == 999);
    }
}

int main()
{
    TestSensoryRoundTrip();
    TestMotorRoundTrip();
    TestMalformedAndInvalidValuesAreRejected();
    TestPythonFieldNamesAreExact();
    TestLatestMailboxRejectsStaleTicks();
    TestMailboxIsThreadSafe();
}
