"""Continuous sensorimotor protocol for biologically driven Bloatfly control.

Protocol v1 remains available as a discrete-action debugging path.  Protocol v2
is the intended FlyBrain boundary: Fallout 4 supplies sensory signals and the
neural side returns continuous motor drives rather than semantic game actions.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import json
from typing import Any


PROTOCOL_VERSION = 2


def _unit(value: float, name: str) -> None:
    if not 0.0 <= value <= 1.0:
        raise ValueError(f"{name} must be between 0 and 1")


def _signed_unit(value: float, name: str) -> None:
    if not -1.0 <= value <= 1.0:
        raise ValueError(f"{name} must be between -1 and 1")


@dataclass(slots=True)
class SensoryFrame:
    tick: int
    dt: float

    # Retinal-like motion channels. Positive values mean stronger motion energy
    # in that part of the visual field, not a pre-decoded turn instruction.
    optic_flow_left: float
    optic_flow_right: float
    optic_flow_up: float
    optic_flow_down: float

    # Expansion / collision channels.  Keep spatial separation so the neural
    # backend can decide escape direction rather than receiving EVADE_LEFT etc.
    looming_left: float
    looming_center: float
    looming_right: float

    # Companion attraction cue.  This is a perceptual salience signal for the
    # player/handler, deliberately not player coordinates or a bearing command.
    familiar_cue_left: float
    familiar_cue_center: float
    familiar_cue_right: float

    # Non-visual internal / nociceptive state.
    damage_signal: float
    health_fraction: float

    def validate(self) -> None:
        if self.tick < 0:
            raise ValueError("tick must be >= 0")
        if self.dt <= 0.0 or self.dt > 1.0:
            raise ValueError("dt must be > 0 and <= 1 second")

        for name in (
            "optic_flow_left",
            "optic_flow_right",
            "optic_flow_up",
            "optic_flow_down",
            "looming_left",
            "looming_center",
            "looming_right",
            "familiar_cue_left",
            "familiar_cue_center",
            "familiar_cue_right",
            "damage_signal",
            "health_fraction",
        ):
            _unit(float(getattr(self, name)), name)

    def to_json(self) -> str:
        self.validate()
        return json.dumps(
            {
                "protocol_version": PROTOCOL_VERSION,
                "type": "sensory_frame",
                **asdict(self),
            },
            separators=(",", ":"),
        )

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "SensoryFrame":
        if payload.get("protocol_version") != PROTOCOL_VERSION:
            raise ValueError("unsupported protocol version")
        if payload.get("type") != "sensory_frame":
            raise ValueError("expected sensory_frame message")

        frame = cls(
            tick=int(payload["tick"]),
            dt=float(payload["dt"]),
            optic_flow_left=float(payload["optic_flow_left"]),
            optic_flow_right=float(payload["optic_flow_right"]),
            optic_flow_up=float(payload["optic_flow_up"]),
            optic_flow_down=float(payload["optic_flow_down"]),
            looming_left=float(payload["looming_left"]),
            looming_center=float(payload["looming_center"]),
            looming_right=float(payload["looming_right"]),
            familiar_cue_left=float(payload["familiar_cue_left"]),
            familiar_cue_center=float(payload["familiar_cue_center"]),
            familiar_cue_right=float(payload["familiar_cue_right"]),
            damage_signal=float(payload["damage_signal"]),
            health_fraction=float(payload["health_fraction"]),
        )
        frame.validate()
        return frame

    @classmethod
    def from_json(cls, text: str) -> "SensoryFrame":
        return cls.from_dict(json.loads(text))


@dataclass(slots=True)
class MotorCommand:
    tick: int

    # Signed continuous rotational / vertical drives.
    yaw: float
    pitch: float
    lift: float

    # Unsigned forward thrust and attack propensity.
    thrust: float
    attack_drive: float

    def validate(self) -> None:
        if self.tick < 0:
            raise ValueError("tick must be >= 0")
        _signed_unit(self.yaw, "yaw")
        _signed_unit(self.pitch, "pitch")
        _signed_unit(self.lift, "lift")
        _unit(self.thrust, "thrust")
        _unit(self.attack_drive, "attack_drive")

    def to_json(self) -> str:
        self.validate()
        return json.dumps(
            {
                "protocol_version": PROTOCOL_VERSION,
                "type": "motor_command",
                **asdict(self),
            },
            separators=(",", ":"),
        )

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "MotorCommand":
        if payload.get("protocol_version") != PROTOCOL_VERSION:
            raise ValueError("unsupported protocol version")
        if payload.get("type") != "motor_command":
            raise ValueError("expected motor_command message")

        command = cls(
            tick=int(payload["tick"]),
            yaw=float(payload["yaw"]),
            pitch=float(payload["pitch"]),
            lift=float(payload["lift"]),
            thrust=float(payload["thrust"]),
            attack_drive=float(payload["attack_drive"]),
        )
        command.validate()
        return command

    @classmethod
    def from_json(cls, text: str) -> "MotorCommand":
        return cls.from_dict(json.loads(text))
