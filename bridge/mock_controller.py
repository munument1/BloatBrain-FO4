"""Minimal controller used before the real FlyBrain backend is connected.

Usage from the repository root:

    python -m bridge.mock_controller

Send one JSONL observation per line on stdin. One JSONL action is written to
stdout. This lets the Fallout 4 integration be tested independently from the
neural simulation.
"""

from __future__ import annotations

import sys

from .protocol import Action, ActionCommand, Observation


def choose_action(obs: Observation) -> ActionCommand:
    """Simple deterministic policy for validating the end-to-end control loop."""

    if obs.health_fraction <= 0.25 or obs.recently_damaged:
        return ActionCommand(obs.tick, Action.EVADE_TARGET, confidence=0.95)

    if not obs.target_visible:
        return ActionCommand(obs.tick, Action.FORWARD, confidence=0.60)

    bearing = obs.target_bearing_deg or 0.0
    elevation = obs.target_elevation_deg or 0.0
    distance = obs.target_distance

    if bearing < -12.0:
        return ActionCommand(obs.tick, Action.TURN_LEFT, confidence=0.85)
    if bearing > 12.0:
        return ActionCommand(obs.tick, Action.TURN_RIGHT, confidence=0.85)

    if elevation > 10.0:
        return ActionCommand(obs.tick, Action.ASCEND, confidence=0.75)
    if elevation < -10.0:
        return ActionCommand(obs.tick, Action.DESCEND, confidence=0.75)

    if distance is not None and distance <= 350.0:
        return ActionCommand(obs.tick, Action.ATTACK, confidence=0.90)

    return ActionCommand(obs.tick, Action.APPROACH_TARGET, confidence=0.80)


def main() -> int:
    for raw_line in sys.stdin:
        line = raw_line.strip()
        if not line:
            continue

        try:
            obs = Observation.from_json(line)
            print(choose_action(obs).to_json(), flush=True)
        except Exception as exc:  # Keep prototype failures visible to caller.
            print(f"BloatBrain mock controller error: {exc}", file=sys.stderr, flush=True)
            return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
