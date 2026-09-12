# FlyBrain Adapter

This directory contains adapters for the external Drosophila/connectome neural backend.

## Boundary

Protocol v1 remains a discrete-action debugging path. The intended biological-control boundary is **protocol v2**:

```text
SensoryFrame
  -> sensory encoder / neural stimulation
  -> neural backend adapter
  -> simulation step(s)
  -> motor-neuron aggregation
  -> MotorCommand
```

The neural side should not receive semantic game orders such as `TURN_LEFT`, `APPROACH_TARGET`, or player world coordinates. Those already solve the behavior problem for the network.

Instead, protocol v2 exposes normalized sensory evidence:

- optic-flow energy: left / right / up / down
- looming expansion: left / center / right
- familiar-player cue salience: left / center / right
- damage signal
- health fraction

and returns continuous drives:

- `yaw`
- `pitch`
- `lift`
- `thrust`
- `attack_drive`

See [`../docs/continuous-control.md`](../docs/continuous-control.md).

## Backend contract

A real backend should expose an interface equivalent to:

```python
class FlyBrainBackend:
    def reset(self) -> None: ...
    def step(self, sensory_input: dict[str, float], dt: float) -> dict[str, float]: ...
```

The returned dictionary represents named motor channels derived from neural activity. The exact neuron populations, aggregation method, dynamics and biological assumptions must be documented alongside each experiment.

## Continuous validation backend

`continuous_mock.py` is deliberately **not** the biological FlyBrain. It exists only to prove that the rest of the stack can operate without discrete game-AI actions.

Even that mock consumes sensory asymmetry and emits continuous motor values. It never receives a ready-made turn direction or player bearing.

This gives the Fallout 4 actuator something realistic to integrate against before the MaleCNS backend is ready.

## Behavioral target

The companion should visibly behave like an imperfect closed-loop insect rather than a waypoint-following drone:

- micro-corrections while hovering
- overshoot and reacquisition
- abrupt saccade-like turns
- looming-driven escape
- occasional face-level hovering and visual-field crossing
- temporary searching instead of GPS-perfect tracking when the player leaves view

These should emerge from sensorimotor dynamics whenever possible, not from an `annoy_player()` state machine.

## Development rule

Do not start by feeding raw Fallout 4 frames into the connectome. First prove that the normalized protocol-v2 sensory vector can drive stable continuous behavior through the full game-control loop. Richer visual stimulation can be added later without redesigning the F4SE side.
