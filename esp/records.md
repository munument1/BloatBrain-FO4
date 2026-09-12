# ESP record specification

This is the authoritative record contract for the first `BloatBrainFO4.esp` prototype.

## Core records

| Type | EditorID | Purpose |
| --- | --- | --- |
| Actor | `BB_FlyCompanion` | Unique friendly Bloatfly companion base |
| Object Reference | `BB_FlyCompanionREF` | Persistent companion placed in `RedRocketExt` |
| XMarkerHeading | `BB_RedRocketHomeMarker` | Dismissed/home location at Red Rocket |
| Keyword | `BB_NeuralControlled` | Marks records owned by BloatBrain |
| Quest | `BB_CompanionQuest` | Start Game Enabled companion state quest |
| Global | `BB_Recruited` | `0` = home/dismissed, `1` = recruited |
| Global | `BB_BridgeOnline` | `0` = fallback AI, `1` = external controller active |
| Message | `BB_RecruitMessage` | Recruit / leave choices |
| Message | `BB_CompanionMenuMessage` | Follow / return-home choices |

## Actor base

Create `BB_FlyCompanion` from a normal Bloatfly base only for art, race, animation and combat assets. Do not reuse hostile faction/aggression settings blindly.

Prototype requirements:

- unique actor base, not a leveled actor
- protected so accidental test deaths do not constantly reset experiments
- non-hostile to the player by default
- remove or override factions that make ordinary Bloatflies hostile to the player
- add `BB_NeuralControlled`
- no dialogue dependency
- activation handled by `BBCompanionRecruitScript`

The first prototype should not depend on the vanilla human companion dialogue framework.

## Placement

Place `BB_FlyCompanionREF` as a **Persistent Reference** in `RedRocketExt`, close to the Red Rocket station but clear of Dogmeat's initial encounter and the mole-rat encounter trigger.

Place `BB_RedRocketHomeMarker` nearby. The dismissed companion's package stack should return it to this marker and sandbox around it.

Do not hard-code the resulting FormID in the DLL. `BB_FlyCompanionREF` is the stable lookup contract.

## Quest and alias

`BB_CompanionQuest`:

- Start Game Enabled
- one forced reference alias pointing to `BB_FlyCompanionREF`
- alias/package stack owns fallback behavior

Recommended package priority from highest to lowest:

1. `BB_NeuralHoldPackage`
   - conditions: `BB_Recruited == 1` and `BB_BridgeOnline == 1`
   - purpose: suppress conflicting travel/follow decisions while the DLL controls neural actions
   - keep this package intentionally conservative; the DLL must not fight a vanilla travel package
2. `BB_FollowFallbackPackage`
   - conditions: `BB_Recruited == 1` and `BB_BridgeOnline == 0`
   - follow the player at a safe creature-follow distance
   - this is the disconnect/crash fallback
3. `BB_ReturnHomePackage`
   - conditions: `BB_Recruited == 0`
   - travel to `BB_RedRocketHomeMarker`
4. `BB_HomeSandboxPackage`
   - conditions: `BB_Recruited == 0`
   - sandbox around the Red Rocket home marker

## Recruitment messages

`BB_RecruitMessage` buttons:

1. `데리고 간다`
2. `그대로 둔다`

`BB_CompanionMenuMessage` buttons:

1. `계속 따라오게 한다`
2. `레드 로켓으로 돌려보낸다`
3. `취소`

Localization can be moved to string files later. For the prototype, message records are sufficient.

## State ownership

`BB_Recruited` is owned by the ESP/Papyrus recruitment flow.

`BB_BridgeOnline` is owned by the F4SE plugin. The DLL should set it only after the external bridge is usable and clear it immediately on timeout/disconnect. This makes the fallback package automatic.

## DLL contract

The F4SE plugin must:

- resolve `BB_FlyCompanionREF` by EditorID after game data is ready
- verify the reference is an Actor
- optionally verify `BB_NeuralControlled`
- sample only this actor
- never scan/control all actors of the Bloatfly race
- update `BB_BridgeOnline` on controller connectivity changes
- keep game-object access on the Fallout 4 game thread
- keep blocking socket I/O on the existing bridge worker thread

## First in-game definition of done

1. New game/save loads with the plugin enabled.
2. The unique Bloatfly is present at Red Rocket.
3. Activating it allows recruitment.
4. Recruited state causes it to follow the player while the external bridge is offline.
5. Starting the bridge flips the controller into neural-control mode.
6. Stopping the bridge causes automatic fallback follow behavior.
7. Dismissal sends the same unique Bloatfly back to Red Rocket.
