"""Shared observation/action protocol for BloatBrain-FO4.

The initial bridge uses newline-delimited JSON (JSONL) so both the Fallout 4
side and the external controller can be debugged independently.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
from enum import Enum
import json
from typing import Any


PROTOCOL_VERSION = 1


class Action(str, Enum):
    IDLE = "IDLE"
    FORWARD = "FORWARD"
    TURN_LEFT = "TURN_LEFT"
    TURN_RIGHT = "TURN_RIGHT"
    ASCEND = "ASCEND"
    DESCEND = "DESCEND"
    APPROACH_TARGET = "APPROACH_TARGET"
    EVADE_TARGET = "EVADE_TARGET"
    ATTACK = "ATTACK"


@dataclass(slots=True)
class Observation:
    tick: int
    target_visible: bool
    target_distance: float | None
    target_bearing_deg: float | None
    target_elevation_deg: float | None
    health_fraction: float
    recently_damaged: bool
    in_combat: bool

    def validate(self) -> None:
        if self.tick < 0:
            raise ValueError("tick must be >= 0")
        if not 0.0 <= self.health_fraction <= 1.0:
            raise ValueError("health_fraction must be between 0 and 1")
        if self.target_distance is not None and self.target_distance < 0.0:
            raise ValueError("target_distance must be >= 0")

    def to_json(self) -> str:
        self.validate()
        payload = {
            "protocol_version": PROTOCOL_VERSION,
            "type": "observation",
            **asdict(self),
        }
        return json.dumps(payload, separators=(",", ":"))

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "Observation":
        if payload.get("protocol_version") != PROTOCOL_VERSION:
            raise ValueError("unsupported protocol version")
        if payload.get("type") != "observation":
            raise ValueError("expected observation message")

        observation = cls(
            tick=int(payload["tick"]),
            target_visible=bool(payload["target_visible"]),
            target_distance=(
                None
                if payload.get("target_distance") is None
                else float(payload["target_distance"])
            ),
            target_bearing_deg=(
                None
                if payload.get("target_bearing_deg") is None
                else float(payload["target_bearing_deg"])
            ),
            target_elevation_deg=(
                None
                if payload.get("target_elevation_deg") is None
                else float(payload["target_elevation_deg"])
            ),
            health_fraction=float(payload["health_fraction"]),
            recently_damaged=bool(payload["recently_damaged"]),
            in_combat=bool(payload["in_combat"]),
        )
        observation.validate()
        return observation

    @classmethod
    def from_json(cls, text: str) -> "Observation":
        return cls.from_dict(json.loads(text))


@dataclass(slots=True)
class ActionCommand:
    tick: int
    action: Action
    confidence: float = 1.0

    def validate(self) -> None:
        if self.tick < 0:
            raise ValueError("tick must be >= 0")
        if not 0.0 <= self.confidence <= 1.0:
            raise ValueError("confidence must be between 0 and 1")

    def to_json(self) -> str:
        self.validate()
        return json.dumps(
            {
                "protocol_version": PROTOCOL_VERSION,
                "type": "action",
                "tick": self.tick,
                "action": self.action.value,
                "confidence": self.confidence,
            },
            separators=(",", ":"),
        )

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "ActionCommand":
        if payload.get("protocol_version") != PROTOCOL_VERSION:
            raise ValueError("unsupported protocol version")
        if payload.get("type") != "action":
            raise ValueError("expected action message")

        command = cls(
            tick=int(payload["tick"]),
            action=Action(payload["action"]),
            confidence=float(payload.get("confidence", 1.0)),
        )
        command.validate()
        return command

    @classmethod
    def from_json(cls, text: str) -> "ActionCommand":
        return cls.from_dict(json.loads(text))
