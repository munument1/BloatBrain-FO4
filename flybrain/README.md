# FlyBrain Adapter

This directory will contain adapters for the external Drosophila/connectome neural backend.

## Boundary

The adapter receives normalized observations from the bridge and returns either:

- motor/action scores, or
- a decoded `ActionCommand`

The rest of the repository should not depend on a particular connectome dataset or simulator.

## Planned layers

```text
Observation
  -> sensory encoder
  -> neural backend adapter
  -> simulation step(s)
  -> motor output aggregation
  -> action decoder
  -> ActionCommand
```

## Phase 1 backend contract

A backend should eventually expose an interface equivalent to:

```python
class FlyBrainBackend:
    def reset(self) -> None: ...
    def step(self, sensory_input: dict[str, float], dt: float) -> dict[str, float]: ...
```

The returned dictionary represents named motor channels or action scores. The exact biological mapping stays backend-specific and must be documented alongside each experiment.

## Development rule

Do not start by feeding raw Fallout 4 frames into the connectome. First prove that a small, normalized sensory vector can drive repeatable behavior through the full game-control loop. Richer visual stimulation can be added later without redesigning the F4SE side.
