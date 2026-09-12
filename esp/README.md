# BloatBrainFO4.esp

This directory defines the game-data side of the project: one unique Bloatfly companion placed at the Red Rocket truck stop and controlled by BloatBrain when the external bridge is available.

## Companion concept

Only one actor is neural-controlled:

- Actor base: `BB_FlyCompanion`
- Persistent placed reference: `BB_FlyCompanionREF`
- Neural marker keyword: `BB_NeuralControlled`
- Home marker: `BB_RedRocketHomeMarker`
- Home cell: `RedRocketExt`

Wild Bloatflies must remain untouched. The F4SE plugin must resolve the exact persistent reference by EditorID and never select actors merely by race.

## Recruitment flow

1. The companion starts at Red Rocket near its home marker.
2. Activating it shows `BB_RecruitMessage`.
3. Accepting sets `BB_Recruited = 1` and starts companion behavior.
4. When `BB_BridgeOnline = 1`, the F4SE/FlyBrain controller owns decisions for the actor.
5. If the bridge disconnects, `BB_BridgeOnline` becomes `0` and the fallback follow package takes over.
6. Dismissing the companion clears `BB_Recruited` and its package stack returns it to Red Rocket.

## Required records

See [`records.md`](records.md) for the complete CK/xEdit record plan.

## Script source

`Scripts/Source/User/BBCompanionRecruitScript.psc` contains the recruitment/dismissal state script. Compile it with the Fallout 4 Papyrus compiler after the records and properties exist.

## Packaging target

The intended final mod layout is:

```text
Data/
├─ BloatBrainFO4.esp
├─ F4SE/Plugins/BloatBrainFO4.dll
└─ Scripts/BBCompanionRecruitScript.pex
```

The ESP should remain independent of Fallout 4 executable runtime versions. Runtime-specific behavior belongs in the F4SE DLL, not in the plugin records.
