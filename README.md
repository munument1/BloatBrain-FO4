# BloatBrain-FO4

Experimental Fallout 4 modding project that drives a single companion Bloatfly with an external **Drosophila-inspired connectome/neural simulation** instead of relying only on vanilla game AI.

> **Status:** early research prototype — Python bridge tested, F4SE transport scaffold builds successfully in Windows CI, the companion ESP contract is defined, and protocol-v2 continuous sensorimotor control is now implemented at the bridge boundary.

## Goal

The mod adds **one unique Bloatfly companion** that can be recruited at the Red Rocket truck stop near Sanctuary. Only that persistent actor is neural-controlled; wild Bloatflies remain completely vanilla.

The intended result is not merely a companion whose decisions come from a network. Its **visible movement should feel insect-like**: continuous micro-corrections, overshoot, abrupt saccade-like turns, looming-driven escape, imperfect reacquisition and occasional face-level hovering should emerge from the closed sensorimotor loop rather than from scripted comedy behavior.

## Companion contract

The ESP and DLL share stable EditorIDs rather than hard-coded FormIDs:

- `BB_FlyCompanion` — unique actor base
- `BB_FlyCompanionREF` — persistent placed companion reference
- `BB_NeuralControlled` — neural-control keyword
- `BB_RedRocketHomeMarker` — dismissed/home marker
- `BB_Recruited` — recruitment state global
- `BB_BridgeOnline` — F4SE controller connectivity global
- `BB_CompanionQuest` — companion state quest

The F4SE plugin must never select actors simply because they use the Bloatfly race.

See [`esp/README.md`](esp/README.md) and [`esp/records.md`](esp/records.md) for the CK/xEdit implementation contract.

## Architecture

```text
Red Rocket
   |
   v
BB_FlyCompanionREF
   |
   | local sensory evidence
   v
F4SE / CommonLibF4 plugin
   |
   | persistent localhost TCP + JSONL
   v
Bridge
   |
   | protocol-v2 SensoryFrame
   v
FlyBrain / MaleCNS backend
   |
   | neural motor activity
   v
continuous motor aggregation
   |
   | yaw / pitch / lift / thrust / attack_drive
   v
F4SE flight actuator
   |
   v
BB_FlyCompanionREF
   |
   +---- changed world becomes the next sensory frame
```

The final neural path deliberately avoids semantic movement orders such as `APPROACH_TARGET` or player GPS coordinates. The network should receive sensory evidence and produce motor drive.

## Repository layout

```text
BloatBrain-FO4/
├─ esp/           # ESP record specification + Papyrus recruitment source
├─ f4se-plugin/   # Fallout 4 integration + WinSock client
├─ bridge/        # v1 debug protocol + v2 continuous sensorimotor protocol
├─ flybrain/      # neural/connectome simulation adapters
├─ configs/       # sensory and motor mappings
├─ tests/         # protocol and transport tests
└─ docs/          # architecture and control notes
```

## Protocols

### Protocol v1 — debugging only

The original discrete action set (`TURN_LEFT`, `APPROACH_TARGET`, `ATTACK`, etc.) remains useful for validating transport, timeouts and basic game integration.

It is **not** the intended final neural-control interface.

### Protocol v2 — intended FlyBrain path

Sensory inputs:

- optic flow: left / right / up / down
- looming expansion: left / center / right
- familiar-player cue: left / center / right
- damage signal
- health fraction

Motor outputs:

- `yaw` in `[-1, 1]`
- `pitch` in `[-1, 1]`
- `lift` in `[-1, 1]`
- `thrust` in `[0, 1]`
- `attack_drive` in `[0, 1]`

See [`docs/continuous-control.md`](docs/continuous-control.md) and [`configs/sensory_motor_v2.example.json`](configs/sensory_motor_v2.example.json).

## Quick bridge test

Python 3.11+:

```powershell
python -m pip install -e ".[dev]"
pytest
```

`flybrain/continuous_mock.py` is a deterministic validation backend for protocol v2. It is not the biological model; it exists so the Fallout 4 side can be built around continuous sensory/motor channels before MaleCNS integration.

The current F4SE worker still uses the protocol-v1 synthetic smoke exchange. Replacing that with real `BB_FlyCompanionREF` sensory sampling and protocol-v2 motor actuation is the next game-side milestone.

The Windows CI workflow compiles the plugin and publishes `BloatBrainFO4.dll` as the `BloatBrainFO4-CI` workflow artifact on successful builds.

## Behavioral target

When neural control is online, the companion should not behave like a waypoint-following drone. Desired observable behavior includes:

- imperfect fixation on the player
- frequent small hover corrections
- sudden direction changes from asymmetric visual motion
- side/vertical escape from looming stimuli
- overshoot and reacquisition
- temporary searching when the player disappears behind geometry
- irregular following distance, including occasionally getting directly in the player's face

The goal is for players to recognize the neural NPC by **how it moves**, not by an icon or visual effect.

## Design principles

1. **Get the loop working before increasing biological complexity.**
2. Control exactly one explicit companion reference; never hijack wild Bloatflies.
3. Keep Fallout 4 integration and neural simulation loosely coupled.
4. Prefer sensory evidence over pre-decoded semantic movement instructions.
5. Prefer continuous motor drive over a small discrete game-AI action menu.
6. Log every sensory frame and motor response so behavior can be reproduced.
7. Treat biological fidelity as an experimental question, not a marketing claim.
8. Never block Fallout 4's main thread on external neural computation or socket I/O.
9. On external-controller failure, immediately fall back to safe vanilla package behavior.

## Planned phases

### Phase 0 — Scaffold

- [x] define protocol-v1 observation/action debugging path
- [x] create local mock controller
- [x] implement persistent localhost TCP bridge
- [x] test multi-tick TCP round trips
- [x] create F4SE/CommonLibF4 transport scaffold
- [x] move blocking bridge I/O to a background worker
- [x] compile the F4SE plugin in Windows CI
- [x] add a synthetic F4SE-to-Python smoke round-trip loop
- [x] define the unique Red Rocket companion ESP contract
- [x] add prototype recruitment/dismissal Papyrus source
- [x] define protocol-v2 continuous sensory/motor messages
- [x] add a continuous validation backend and tests
- [x] mirror protocol-v2 types, JSON validation and latest-value mailboxes in pure C++

### Phase 1 — Fallout 4 companion control loop

- [ ] build `BloatBrainFO4.esp` from the record specification
- [ ] place `BB_FlyCompanionREF` at Red Rocket and verify recruitment/dismissal
- [ ] load the F4SE plugin in Fallout 4 and verify the smoke loop in logs
- [ ] resolve `BB_FlyCompanionREF` after game data is ready
- [ ] sample protocol-v2 sensory channels on the game thread
- [ ] receive and validate matching continuous `MotorCommand`s
- [ ] set/clear `BB_BridgeOnline` on connectivity changes
- [ ] constrain vanilla decision-making while neural control is online
- [ ] apply continuous yaw/pitch/lift/thrust safely to the flying actor
- [ ] gate the Bloatfly attack animation/projectile from `attack_drive`
- [ ] recover safely on stale command/timeout/disconnect

### Phase 2 — FlyBrain integration

- [ ] load/connect to the selected Drosophila neural model
- [ ] map protocol-v2 sensory channels to neural stimulation
- [ ] aggregate candidate descending/motor-neuron populations
- [ ] map neural activity directly to continuous motor channels
- [ ] remove the continuous mock from the production control path

### Phase 3 — Experiments

- compare fallback AI vs FlyBrain behavior
- measure trajectory, hover jitter, saccades and reacquisition behavior
- test different sensory mappings
- record neural/motor activity alongside game trajectories
- explore reward/modulatory signals only after the base loop is stable

## Important note

This project does **not** claim to reproduce a conscious fly or perfectly simulate a biological nervous system. A connectome provides wiring information; simulation dynamics, sensory encoding, neuromodulation, body dynamics and motor decoding all require modeling assumptions.

## License

No license has been selected yet.
