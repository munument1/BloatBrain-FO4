"""Deterministic continuous controller used only to validate protocol-v2 plumbing.

This is not the biological FlyBrain.  It intentionally reacts to sensory
channels instead of semantic target commands so the Fallout 4 side can be
built around the correct sensorimotor boundary before MaleCNS is integrated.
"""

from __future__ import annotations

from bridge.protocol_v2 import MotorCommand, SensoryFrame


def _clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def choose_motor(frame: SensoryFrame) -> MotorCommand:
    frame.validate()

    # Oppose asymmetric visual motion for basic gaze/flight stabilization.
    flow_yaw = frame.optic_flow_left - frame.optic_flow_right
    flow_pitch = frame.optic_flow_down - frame.optic_flow_up

    # Treat the familiar player cue as attraction without giving the controller
    # player coordinates or a ready-made bearing.
    familiar_yaw = frame.familiar_cue_right - frame.familiar_cue_left
    familiar_strength = max(
        frame.familiar_cue_left,
        frame.familiar_cue_center,
        frame.familiar_cue_right,
    )

    # Spatial looming produces escape bias away from the expanding side.
    looming_yaw = frame.looming_left - frame.looming_right
    looming_strength = max(
        frame.looming_left,
        frame.looming_center,
        frame.looming_right,
    )

    yaw = _clamp(
        (0.65 * flow_yaw) + (0.55 * familiar_yaw) + (1.10 * looming_yaw),
        -1.0,
        1.0,
    )
    pitch = _clamp(0.70 * flow_pitch, -1.0, 1.0)

    # Central looming causes a sharp vertical escape component. Damage adds an
    # independent arousal-like boost without prescribing a direction.
    lift = _clamp(
        (0.90 * frame.looming_center) + (0.45 * frame.damage_signal),
        -1.0,
        1.0,
    )

    # The mock advances when a familiar cue is present, but reduces thrust when
    # something rapidly expands in the visual field.
    thrust = _clamp(
        0.20 + (0.70 * familiar_strength) - (0.75 * looming_strength),
        0.0,
        1.0,
    )

    # Attack drive is deliberately only a scalar propensity. The game side may
    # gate actual projectile firing on cooldown/animation constraints.
    attack_drive = _clamp(
        (0.60 * frame.familiar_cue_center)
        + (0.25 * frame.damage_signal)
        - (0.80 * looming_strength),
        0.0,
        1.0,
    )

    return MotorCommand(
        tick=frame.tick,
        yaw=yaw,
        pitch=pitch,
        lift=lift,
        thrust=thrust,
        attack_drive=attack_drive,
    )
