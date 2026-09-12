import pytest

from bridge.protocol_v2 import MotorCommand, SensoryFrame
from flybrain.continuous_mock import choose_motor


def make_frame(**overrides):
    values = dict(
        tick=7,
        dt=0.05,
        optic_flow_left=0.2,
        optic_flow_right=0.2,
        optic_flow_up=0.1,
        optic_flow_down=0.1,
        looming_left=0.0,
        looming_center=0.0,
        looming_right=0.0,
        familiar_cue_left=0.0,
        familiar_cue_center=0.8,
        familiar_cue_right=0.0,
        damage_signal=0.0,
        health_fraction=1.0,
    )
    values.update(overrides)
    return SensoryFrame(**values)


def test_sensory_frame_round_trip():
    frame = make_frame(looming_right=0.4)
    decoded = SensoryFrame.from_json(frame.to_json())
    assert decoded == frame


def test_motor_command_round_trip():
    command = MotorCommand(
        tick=3,
        yaw=-0.3,
        pitch=0.2,
        lift=0.5,
        thrust=0.75,
        attack_drive=0.1,
    )
    decoded = MotorCommand.from_json(command.to_json())
    assert decoded == command


def test_right_looming_biases_escape_left():
    command = choose_motor(
        make_frame(
            familiar_cue_center=0.0,
            looming_right=1.0,
        )
    )
    assert command.yaw < 0.0
    assert command.thrust < 0.5


def test_familiar_cue_on_right_biases_turn_right():
    command = choose_motor(
        make_frame(
            familiar_cue_center=0.0,
            familiar_cue_right=1.0,
        )
    )
    assert command.yaw > 0.0


def test_central_looming_triggers_vertical_escape():
    command = choose_motor(
        make_frame(
            familiar_cue_center=0.0,
            looming_center=1.0,
        )
    )
    assert command.lift > 0.5


def test_protocol_rejects_out_of_range_sensory_input():
    with pytest.raises(ValueError):
        make_frame(looming_center=1.5).validate()


def test_protocol_rejects_out_of_range_motor_output():
    with pytest.raises(ValueError):
        MotorCommand(
            tick=0,
            yaw=1.2,
            pitch=0.0,
            lift=0.0,
            thrust=0.5,
            attack_drive=0.0,
        ).validate()
