# BloatBrain-FO4

Experimental Fallout 4 modding project that drives a single companion Bloatfly with an external **Drosophila-inspired connectome/neural simulation** instead of relying only on vanilla game AI.

> **Status:** early research prototype — Python bridge tested, F4SE transport scaffold builds successfully in Windows CI, and the companion ESP contract is now defined.

## Goal

The mod adds **one unique Bloatfly companion** that can be recruited at the Red Rocket truck stop near Sanctuary. Only that persistent actor is neural-controlled; wild Bloatflies remain completely vanilla.

The intended player-facing loop is:

1. Find the unique Bloatfly at Red Rocket.
2. Activate it and recruit it.
3. While the external bridge is online, BloatBrain/FlyBrain chooses its movement and combat actions.
4. If the bridge disconnects, a safe fallback follow package takes over.
5. Dismissing the companion sends the same actor back to Red Rocket.

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
BB_FlyCompanionREF (persistent unique actor)
   |
   | game-state observations
   v
F4SE / CommonLibF4 plugin
   |
   | persistent localhost TCP + JSONL
   v
Bridge (127.0.0.1:8765)
   |
   | sensory encoding
   v
FlyBrain simulation
   |
   | motor activity / action scores
   v
Action decoder
   |
   v
F4SE plugin -> BB_FlyCompanionREF behavior
```

## Repository layout

```text
BloatBrain-FO4/
├─ esp/           # ESP record specification + Papyrus recruitment source
├─ f4se-plugin/   # Fallout 4 integration + WinSock client
├─ bridge/        # TCP server and observation/action protocol
├─ flybrain/      # neural/connectome simulation adapter
├─ configs/       # sensory and motor mappings
├─ tests/         # protocol and TCP round-trip tests
└─ docs/          # architecture and research notes
```

## Quick bridge test

Python 3.11+:

```powershell
python -m pip install -e ".[dev]"
python -m bridge.tcp_server
```

The bridge listens on `127.0.0.1:8765`. It currently routes observations to the deterministic mock controller so the Fallout 4 integration can be tested before the real neural backend exists.

Run the automated protocol/TCP tests with:

```powershell
pytest
```

The F4SE worker currently sends a synthetic protocol-v1 observation once per second and logs the returned action. This temporary smoke loop will be replaced by observations from `BB_FlyCompanionREF`.

The Windows CI workflow compiles the plugin and publishes `BloatBrainFO4.dll` as the `BloatBrainFO4-CI` workflow artifact on successful builds.

See [`f4se-plugin/README.md`](f4se-plugin/README.md) for the CommonLibF4/XMake setup.

## MVP action space

The first prototype uses a deliberately small discrete action set:

- `IDLE`
- `FORWARD`
- `TURN_LEFT`
- `TURN_RIGHT`
- `ASCEND`
- `DESCEND`
- `APPROACH_TARGET`
- `EVADE_TARGET`
- `ATTACK`

## MVP observations

The Fallout 4 side should initially expose only information that is easy to validate:

- target visible / not visible
- target distance
- target bearing relative to the Bloatfly
- relative target elevation
- Bloatfly health fraction
- recent damage event
- combat state

Later versions can experiment with richer visual, spatial, and reward signals.

## Design principles

1. **Get the loop working before increasing biological complexity.**
2. Control exactly one explicit companion reference; never hijack wild Bloatflies.
3. Keep Fallout 4 integration and neural simulation loosely coupled.
4. Make sensory/motor mappings configurable instead of hard-coding experiments.
5. Log every observation and selected action so behavior can be reproduced.
6. Treat biological fidelity as an experimental question, not a marketing claim.
7. Never block Fallout 4's main thread on external neural computation or socket I/O.
8. On external-controller failure, immediately fall back to safe vanilla package behavior.

## Planned phases

### Phase 0 — Scaffold

- [x] define observation/action protocol
- [x] create local mock controller
- [x] implement persistent localhost TCP bridge
- [x] test multi-tick TCP round trips
- [x] create F4SE/CommonLibF4 transport scaffold
- [x] move blocking bridge I/O to a background worker
- [x] compile the F4SE plugin in Windows CI
- [x] add a synthetic F4SE-to-Python smoke round-trip loop
- [x] define the unique Red Rocket companion ESP contract
- [x] add prototype recruitment/dismissal Papyrus source

### Phase 1 — Fallout 4 companion control loop

- [ ] build `BloatBrainFO4.esp` from the record specification
- [ ] place `BB_FlyCompanionREF` at Red Rocket and verify recruitment/dismissal
- [ ] load the F4SE plugin in Fallout 4 and verify the smoke loop in logs
- [ ] resolve `BB_FlyCompanionREF` after game data is ready
- [ ] read target/game state on the game thread
- [ ] serialize protocol-v1 observations from the actor
- [ ] receive and validate matching action commands
- [ ] set/clear `BB_BridgeOnline` on connectivity changes
- [ ] constrain vanilla decision-making while neural control is online
- [ ] apply returned actions
- [ ] recover safely on timeout/disconnect

### Phase 2 — FlyBrain integration

- [ ] load/connect to the selected Drosophila neural model
- [ ] map game observations to sensory stimulation
- [ ] aggregate candidate motor-neuron activity
- [ ] decode activity into the MVP action space

### Phase 3 — Experiments

- compare fallback AI vs FlyBrain behavior
- test different sensory mappings
- record trajectories and action distributions
- explore reward/modulatory signals only after the base loop is stable

## Important note

This project does **not** claim to reproduce a conscious fly or perfectly simulate a biological nervous system. A connectome provides wiring information; simulation dynamics, sensory encoding, neuromodulation, and motor decoding all require modeling assumptions.

## License

No license has been selected yet.
