# CTR Native Modding Architecture

**Scope:** The structural changes that make the codebase moddable, the runtime mod
interface, and the mod package format.

This document specifies *what to build*, not a line-by-line implementation. It is
deliberately opinionated about structure so that the system stays maintainable as
the moddable surface grows and as a scripting engine is layered on later.

## 1. Goals and non-goals

### Goals

- Mod authors can change game behavior **without editing or recompiling game
  source**. They build against a stable interface only.
- Mods are **redistributable packages** with declarative metadata: identity,
  versioned dependencies, and capabilities.
- **Multiple mods coexist** in one running game, with deterministic load order and
  automatic conflict detection.
- One interface serves **both native (compiled) mods and a future scripting
  engine**. The scripting layer binds to the same surface; it does not duplicate it.
- The mod system **compiles out completely** when disabled, leaving the
  retail/decompilation build path byte-identical.

### Non-goals

- Intercepting *arbitrary* functions at runtime. The moddable surface is an
  explicit, curated, growing set of seams (see §2.2). This is a feature, not a
  limitation: the surface is auditable and documented.
- Sandboxing untrusted native mods. Native mods run in-process with full
  privileges. Untrusted-mod safety is a property of the future scripting tier, not
  the native tier.

## 2. Where we are

Two facts dominate every design decision.

**It is a unity build.** `main.c` `#include`s `game_includes.h`, which `#include`s
every `game/*.c` file into a single translation unit, compiled at `-O2` into one
static executable (`CMakeLists.txt:83-105`). There is no dynamic linking of game
code, and the compiler inlines aggressively. 

**Consequence:** functions cannot be reliably detoured at runtime, because most of them are inlined 
and have no standalone address. The moddable surface must be carved explicitly into the source.

### 2.1 The frame spine

`CTR_Main` (`game/MAIN/MainMain.c:56`) is a state machine. The live frame is
`case 3`, whose order is:

```
GAMEPAD_ProcessAnyoneVars   (input)
MainFrame_ResetDB           (frame start / draw-buffer reset)
MainFrame_GameLogic         (simulation: the 13-stage driver pipeline + audio)
MainFrame_RenderFrame       (render)
```

These named boundaries are where lifecycle and event points attach (§4.2, §4.3).

### 2.2 The moddable surface

Define one term used throughout this document:

> **Seam** — an explicit, named point in game source where mods can observe or
> alter behavior. A seam is either an **event** (§5.2) or a **hook** (§5.3).

The set of seams *is* the moddable API surface. It starts small, is curated by
engine developers, and grows on demand. Everything not behind a seam stays inlined
and fast.

**Events are the primary seam type; hooks are the exception.** The design target is
that events cover most modding needs. A hook replaces a whole function
and therefore bypasses any events that function emits, so the two cannot
meaningfully coexist on the same function. The engine is expected to grow its
*event* coverage as the default response to a new modding need, and to reach for a
hook only when a behavior genuinely cannot be expressed as an observable/mutable/
vetoable point. See principle 8 (§3) and §4.4.

## 3. Design principles

These are binding constraints on all new mod-system code (`include/mod/`,
`platform/`, and the seam macros). They exist because this subsystem will be
long-lived and must stay testable.

1. **One contract, two consumers.** A single C interface — the `ModApi` table
   (§6) — is the entire surface mods see. Native DLL mods receive it by pointer.
   The future scripting engine binds the *same* table. Never build a feature for
   one consumer that the other cannot reach.

2. **Mods depend on the contract, not on game internals.** Two kinds of coupling
   are forbidden because they break shipped mod binaries: mods do not see game
   *struct layouts* (field offsets are baked into the binary and break on any
   refactor), and do not link game *symbols* (functions or globals, which would
   force the executable to export its internals). Instead they use opaque handles
   (`ModDriverHandle`, …) and accessor functions, so internals refactor freely
   behind them. *Compile-time constants are the exception:* a gameplay flag such as
   a boost-type bit is an inlined value with no layout or link dependency, so such
   constants are part of the contract — but they are exposed through a **generated
   mirror** in the stable mod headers (§5.4), never by including a game header.
   Mods thus neither reinvent these values by hand nor import game-internal headers
   to get them. (A raw-access escape hatch exists for power mods, §6.4, gated by an
   explicit unstable-ABI opt-in.)

3. **No abbreviations in new identifiers.** Existing game code keeps its names
   (`functionTick`, `gameTracker`). All *new* mod-system identifiers spell words
   out: `ModEvent_Subscribe`, not `ModEvt_Sub`; `dependencyResolver`, not
   `depRes`. Public functions are `ModNoun_Verb` (`ModLoader_Initialize`).

4. **Small, single-responsibility, unit-testable functions.** Parsing, dependency
   resolution, conflict detection, and load-order computation are **pure functions
   over plain data** — no file I/O, no `LoadLibrary`, no globals. I/O and OS calls
   live only at the edges and are injected (principle 5).

5. **Dependency injection at every OS boundary.** The loader depends on a
   `ModHostServices` interface (§5.7), not directly on `fopen`/`LoadLibrary`/
   `printf`. Production wires real implementations; tests wire in-memory fakes.
   This is what makes the loader logic testable without a disk or a game.

6. **Compile out when disabled.** Every seam and every lifecycle call sits behind
   `#ifdef CTR_MODDING` and reduces to nothing when the feature is off, preserving
   the non-native decompilation build.

7. **Single source of truth.** Each event and hook is declared once in a manifest
   table (§5.4). Event identifiers, context typedefs, dispatch stubs, generated
   documentation, and future script bindings are all derived from it, so they
   cannot drift.

8. **Events first; hooks are a last resort.** Events are the primary modding
   mechanism and should cover the large majority of use cases. Hooks
   (whole-function replacement) exist only for behavior no event can express. The
   two are
   *fundamentally in tension*: a hook that replaces a function silently bypasses
   every event emitted inside it, breaking any mod that listens there. Therefore
   the engine's job is to **emit enough well-placed events that hooks are rarely
   needed**, and the bar for adding a hook is high (§4.4, §5.3). When a use case
   can be met by an event, it must be — adding a hook where an event would do is a
   defect.

## 4. Getting the codebase into a moddable state

This section is the engine-side work. It is independent of any individual mod.

### 4.1 New module: `include/mod/`

A self-contained module that does **not** depend on unity include order and pulls
in no game headers (only opaque forward declarations).

| File | Responsibility |
| --- | --- |
| `mod_abi.h` | `MOD_API_VERSION` constant, calling-convention macros, fixed-width handle typedefs. |
| `mod_api.h` | The `ModApi` table and opaque handle types — the entire surface a mod sees. |
| `mod_host_services.h` | The injected `ModHostServices` interface (§5.7). |
| `mod_loader.h` / `mod_loader.c` | Discovery, manifest parsing, dependency resolution, conflict detection, library loading, lifecycle. Logic is pure; I/O is injected. |
| `mod_event.h` / `mod_event.c` | Event registry and the three emit paths. |
| `mod_hook.h` / `mod_hook.c` | Hook registry: override and chaining. |
| `mod_manifest.h` / `mod_manifest.c` | `package.json` model + pure parser/validator. |
| `mod_events.def`, `mod_hooks.def` | X-macro seam manifests (single source of truth). |
| `mod_constants.def` | X-macro mirror of gameplay constants exposed to mods, static-asserted against the game's own definitions (§5.4). |

`mod_api.h` and `mod_abi.h` are the only headers a mod package ships against.

### 4.2 Lifecycle insertion points

A small fixed set of calls added to files the project already owns:

| Call | Location | Purpose |
| --- | --- | --- |
| `ModLoader_Initialize` | `main.c`, after `Platform_Init` (`main.c:214/217`) | Discover, validate, order, and load mods. |
| `ModLoader_Shutdown` | `main.c`, before `Platform_Shutdown` (`main.c:246`) | Unload in reverse order. |
| `ModLoader_BeginFrame` | `CTR_Main` case 3, before `MainFrame_GameLogic` (`game/MAIN/MainMain.c`) | Per-frame hook for mods and the script VM. |
| `ModLoader_EndFrame` | `CTR_Main` case 3, after `MainFrame_RenderFrame` | Symmetric end-of-frame point. |

There is precedent for registered host callbacks here: `VSyncCallback` /
`DrawSyncCallback` in `game/MAIN/MainDrawCb.c`.

### 4.3 Event seam mechanism

Events are emitted at phase boundaries. Three macros, all no-ops when no listener
is subscribed and fully removed when `CTR_MODDING` is undefined:

```c
MOD_EMIT(EVENT_RACE_START, &context);               // notification: void
MOD_EMIT_MUTABLE(EVENT_VEHICLE_CALCULATE, &context); // listeners mutate context
if (MOD_EMIT_VETO(EVENT_PICKUP_USE, &context))       // veto: any listener can cancel
    return;
```

- **Notification** — observers receive a `const` context; return value ignored.
- **Mutable** — observers receive a writable context and may alter exposed fields;
  the game reads the context back after emission.
- **Veto** — observers return a verdict; if any vetoes, the macro yields true and
  the call site skips the default behavior.

A per-event subscriber count gates a fast path (`if (subscriberCount[id]) …`), so
an unsubscribed event costs one predictable branch.

### 4.4 Hook / proxy mechanism (exceptional)

Hooks are the **last-resort** mechanism (principle 8, §3), for behavior no event can
express. Replacing a function bypasses any events emitted inside it, so a function
that already has event seams should not also be hooked. Prefer adding or widening an
event over marking a function hookable.

Two complementary techniques; neither edits call sites.

**Marked functions.** A macro converts a chosen function's *definition* into a
hookable indirection. Callers keep calling it by name; the call now routes through
a replaceable pointer.

```c
HOOK_FUNCTION(void, VehFrame, (struct Driver *driver)) { /* original body */ }
```

Mods replace and chain:

```c
ModHook_Install(modApi, HOOK_ID_VEHICLE_FRAME, myVehicleFrame, &previous);
// inside myVehicleFrame: call `previous(driver)` to invoke the original ("super").
```

Cost: a marked function loses inlining. Acceptable for a curated set; the macro
keeps that set greppable and auditable.

**Behavior-pointer wrapping.** Because object behavior is already function-pointer
driven (§2), route `INSTANCE_BirthWithThread` (`game/INSTANCE.c:105`) and
`ThTick_SetAndExec` through the hook registry. Mods then wrap behavior **by object
type** with no new call-site indirection — covering most gameplay cheaply.

### 4.5 Asset override

`NativeAssets_ResolvePath` (`platform/native_assets.c:633`) already resolves every
game file before `BIGFILE.BIG`. Insert a `mods/<id>/assets/` search ahead of it,
ordered by mod load order. This delivers data/texture/model/audio mods with no
further source changes and is the basis for the asset-conflict rules in §7.4.

### 4.6 Build-system changes

- Mods are **separate 32-bit shared libraries** (`-m32`, MinGW-compatible),
  matching the host (`CMakeLists.txt:10-13`). Each exports exactly one symbol,
  `ModRegister` (§6.1).
- The host **exports no symbols**. It injects the `ModApi` pointer at registration,
  so mods need nothing from the executable's symbol table. This keeps the clean
  static link (`CMakeLists.txt:104`) intact and is the reason the ABI is a table,
  not a link-time contract.
- Add a `CTR_MODDING` build option (default on for native, off for the
  decompilation path).
- A separate `mod_loader_tests` target compiles the pure loader/parser/resolver
  sources against fake `ModHostServices` (§5.7). No game code, no SDL.
- The scripting engine (e.g. Python) is vendored under `externals/` behind its own
  build flag.

### 4.7 Dependency injection and testability

The loader's OS dependencies are an injected interface so its logic is unit
testable. Sketch:

```c
typedef struct ModHostServices
{
    /* Filesystem */
    int  (*directoryListEntries)(void *context, const char *path,
                                 ModStringList *outEntries);
    int  (*fileReadAll)(void *context, const char *path, ModByteBuffer *outBytes);

    /* Dynamic libraries */
    int  (*libraryLoad)(void *context, const char *path, ModLibraryHandle *outHandle);
    int  (*librarySymbol)(void *context, ModLibraryHandle library,
                          const char *symbolName, void **outSymbol);
    void (*libraryUnload)(void *context, ModLibraryHandle library);

    /* Diagnostics */
    void (*logMessage)(void *context, ModLogLevel level, const char *message);

    void *context; /* opaque, passed back to every callback (no globals) */
} ModHostServices;
```

The functions that carry the real logic take plain data and are individually
testable:

```c
/* Pure: parse + validate a manifest from bytes already in memory. */
ModManifestResult ModManifest_Parse(ModStringView json, ModManifest *outManifest);

/* Pure: given parsed manifests, produce a load order or a dependency error. */
ModResolveResult  ModLoader_ResolveLoadOrder(const ModManifest *manifests,
                                             int manifestCount,
                                             ModLoadOrder *outOrder);

/* Pure: given the ordered set, report conflicts (§7.4). */
ModConflictReport ModLoader_DetectConflicts(const ModManifest *ordered,
                                            int count);
```

`ModLoader_Initialize` is the thin imperative shell that wires these together using
the injected services.

## 5. The runtime mod interface

### 5.1 Registration and lifecycle

Every native mod exports one function:

```c
/* Implemented by the mod. Called once after the host validates the package. */
MOD_EXPORT int ModRegister(const ModApi *api, ModContextHandle self);
```

- `api` is the host-provided table (§6). The mod stores it and uses it for all
  subsequent calls.
- `self` identifies this mod for ownership tracking (which hooks/events it owns,
  for unload and conflict reporting).
- Return zero on success. A non-zero return aborts loading this mod cleanly.

Optional lifecycle callbacks a mod may register through the `api`:
`onEnable`, `onDisable`, `onFrameBegin`, `onFrameEnd`, `onShutdown`.

**Manifest as contract.** A mod may only subscribe to events, install hooks, and
override assets that its `package.json` declares (§7.3). Registration calls outside
the declared capability set are rejected and logged. The manifest is therefore a
verifiable description of what the mod can touch — the foundation of compatibility
analysis (§7.4).

### 5.2 Events API

```c
ModSubscriptionHandle ModEvent_Subscribe(const ModApi *api,
                                         ModContextHandle self,
                                         ModEventId eventId,
                                         ModEventCallback callback,
                                         void *userData);

void ModEvent_Unsubscribe(const ModApi *api, ModSubscriptionHandle subscription);
```

`callback` receives a typed, per-event context (generated from the seam manifest,
§5.4) and returns a `ModEventVerdict`. For notification and mutable events the
verdict is ignored; for veto events `MOD_EVENT_VERDICT_CANCEL` cancels the default
behavior. Listener order follows mod load order (§7.5).

### 5.3 Hooks API

```c
int  ModHook_Install(const ModApi *api, ModContextHandle self,
                     ModHookId hookId, void *replacement, void **outPrevious);
void ModHook_Remove(const ModApi *api, ModHookHandle hook);
```

`outPrevious` returns the prior target so the replacement can chain to it. A hook
is **exclusive**: a second mod installing the same `hookId` is a conflict unless
the first explicitly published a chained interface (§7.4).

Hooks are intended to be rare (principle 8, §3). A mod that hooks a function whose
behavior is also exposed through events takes responsibility for those events: by
replacing the function it suppresses them unless it re-emits or chains to the
original. This is precisely why the engine favors event coverage — a mod ecosystem
built on events composes; one built on hooks collides.

### 5.4 Seam manifests and generated contexts

Each event and hook is declared once in `mod_events.def` / `mod_hooks.def` using
X-macros, for example:

```c
/* mod_events.def */
MOD_EVENT(RACE_START,        ModEventRaceStartContext,        notification)
MOD_EVENT(VEHICLE_CALCULATE, ModEventVehicleCalculateContext, mutable)
MOD_EVENT(PICKUP_USE,        ModEventPickupUseContext,        veto)
```

The build expands this table into: the `ModEventId` enum, the context struct
forward declarations, dispatch stubs, the human-readable seam catalog, and (later)
the scripting bindings. No parallel hand-maintained lists exist.

**Generated constants.** Gameplay values that are part of the mod contract — boost
type bits, kart states, action flags, and similar — are declared once in
`mod_constants.def` and expanded into stable `MOD_*` constants in the mod headers.
Each generated constant carries a `CTR_STATIC_ASSERT` tying it to the game's own
definition (for example `MOD_BOOST_TYPE_TURBO_PAD == TURBO_PAD`), so the build fails
loudly if the two ever drift. Mods therefore reuse the engine's exact values without
including any game-internal header, and the scripting tier (in the future) binds the same
generated set rather than a hand-copied one — which is the reason a generated mirror
is preferred over a shared C header: the script environment cannot consume a C
header at all, so the constants must exist as a language-neutral, generated source
regardless.

### 5.5 Versioning

`ModApi` carries `MOD_API_VERSION` (semantic). At registration the host compares it
to the version the package declares (`apiVersion`, §7.2). Major-version mismatch is
rejected; minor differences are allowed (the table only grows). New table entries
are appended, never reordered, so older mods keep working.

### 5.6 Naming conventions (new code only)

- Public functions: `ModNoun_Verb` — `ModLoader_Initialize`, `ModEvent_Subscribe`.
- Types: `ModNounRole` — `ModHostServices`, `ModEventCallback`.
- Handles: `ModNounHandle`.
- Constants/enums: `MOD_NAMESPACE_VALUE` — `MOD_EVENT_VERDICT_CANCEL`.
- No abbreviations in any of the above.

## 6. The `ModApi` table

A struct of function pointers — the entire surface a mod sees. It is the
dependency the mod receives by injection rather than linking. Illustrative shape:

```c
typedef struct ModApi
{
    unsigned int version; /* MOD_API_VERSION the host was built with */

    /* Registration-time services */
    ModSubscriptionHandle (*eventSubscribe)(ModContextHandle self, ModEventId,
                                            ModEventCallback, void *userData);
    void                  (*eventUnsubscribe)(ModSubscriptionHandle);
    int                   (*hookInstall)(ModContextHandle self, ModHookId,
                                         void *replacement, void **outPrevious);
    void                  (*hookRemove)(ModHookHandle);

    /* Accessors — opaque-handle reads/writes (§6.4) */
    int  (*driverGetVelocity)(ModDriverHandle driver, ModVector3 *outVelocity);
    int  (*driverSetVelocity)(ModDriverHandle driver, const ModVector3 *velocity);
    /* … grows as seams expose more state … */

    /* Diagnostics and config */
    void (*logMessage)(ModContextHandle self, ModLogLevel, const char *message);
    int  (*configReadInt)(ModContextHandle self, const char *key, int fallback);
} ModApi;
```

### 6.4 Accessors and the raw-access escape hatch

The default surface is opaque handles plus accessor functions, so internals stay
refactorable (principle 2). The game's PSX-layout-locked structs (asserted in
`include/namespace_Main.h`, e.g. `sizeof(struct GameTracker)`) are unusually
stable, so a power mod may opt into raw struct access by declaring
`"unstableAbi": true` in its manifest and including the game headers directly. Such
a mod is flagged incompatible across engine versions and excluded from the stable
compatibility guarantees of §7.4.

## 7. Mod packaging

### 7.1 Archive layout

A mod is a ZIP archive:

```
double-reserves.zip
  package.json            required — metadata and capabilities
  double_reserves.dll     optional — 32-bit native entry point (omit for asset-only mods)
  assets/                 optional — files overriding resolved game paths
    textures/...
  scripts/                optional — reserved for the scripting tier
```

The loader extracts to a managed `mods/<id>/` directory. Asset-only mods need no
native library and no toolchain.

The single `.dll`/`.so` is a *build artifact*, not a limit on authorship: a mod's
source may be any number of `.c`/`.h` files organized however the author prefers,
linked into one shared library. The only requirement is that the library export
exactly one symbol, `ModRegister` (§5.1); all other code is internal to the mod.
The same applies to the scripting tier — a script mod may span multiple files with
its own module structure, and the manifest names the entry point.

### 7.2 `package.json` schema

```jsonc
{
  "id": "com.example.double-reserves",       // required, reverse-DNS, unique
  "name": "Double Reserves",                 // required, display name
  "version": "1.2.0",                        // required, semantic version
  "apiVersion": "1.0",                       // required, ModApi version targeted
  "author": "Example Author",                // optional
  "description": "Doubles reserves gained.", // optional
  "entryPoint": "double_reserves.dll",       // optional; omit => asset-only mod

  "dependencies": [                          // optional
    { "id": "com.example.core", "version": ">=2.0.0 <3.0.0" }
  ],

  "capabilities": {                         // required; the enforced contract (§5.1)
    "events": [
      { "id": "race.start",            "mode": "notification" },
      { "id": "vehicle.calculate",     "mode": "mutable" },
      { "id": "pickup.use",            "mode": "veto" }
    ],
    "hooks": [ "vehicle.frame" ],
    "assetOverrides": [ "textures/kart_crash.tex" ]
  },

  "affectsSimulation": true,             // optional, default derived (§8)
  "unstableAbi": false                   // optional, default false (§6.4)
}
```

The parser (`ModManifest_Parse`) is pure and fully unit-testable: bytes in, a
validated `ModManifest` or a structured error out.

### 7.3 Capabilities as the enforced contract

`capabilities` is not documentation; it is enforced. At registration the host
rejects any subscription, hook, or asset override the manifest did not declare.
This guarantees the manifest is an accurate, machine-checkable description of the
mod's reach — which makes compatibility analysis sound rather than advisory.

### 7.4 Compatibility flags and conflict detection

Each mod is assigned an **effective compatibility class**, derived from its declared
capabilities (not hand-set):

| Class | Condition | Combination behavior |
| --- | --- | --- |
| `Pure` | Only notification events and/or asset overrides on non-shared paths | Commutative; combines freely with anything. |
| `OrderSensitive` | Subscribes to any **mutable** or **veto** event | Combinable, but outcome depends on load order (§7.5); flagged for author awareness. |
| `Exclusive` | Installs one or more **hooks**, or **replaces** an asset another mod also replaces | Conflicts with any other mod claiming the same hook id or asset path. |

`ModLoader_DetectConflicts` (pure function) computes, over the ordered mod set:

- **Hard conflict** — two mods install the same `hookId`, or override the same
  asset path. Reported; resolution is to disable one or to declare an explicit
  dependency that establishes a chain/override order.
- **Soft conflict** — overlapping mutable/veto subscriptions on the same event.
  Permitted; resolved deterministically by load order and surfaced as a warning so
  authors can declare ordering dependencies if needed.

Because the inputs are the declared capabilities, the report is deterministic and
testable without loading any code.

### 7.5 Dependency resolution and load order

- Dependencies are `(id, semver-range)` pairs. Resolution is a topological sort
  over the dependency graph (`ModLoader_ResolveLoadOrder`, pure).
- Cycles and unsatisfied/incompatible versions are hard errors reported before any
  library is loaded.
- Load order is the topological order; ties are broken by a stable, declared
  priority then by id, so ordering is reproducible.
- Event listener order and asset-override precedence both follow load order, which
  is what makes `OrderSensitive` outcomes deterministic.

## 8. Working example: "Double Reserves" mod

This example demonstrates the whole pipeline end to end. 

**Goal:** every powerslide, turbo-pad, and hangtime boost grants double its usual reserves. It is
intentionally small — once the codebase is adjusted, this is the entire mod.

### 8.1 The relevant game mechanic

All three boosts funnel through one function:

```c
/* game/Vehicle/VehFire.c:70 */
void VehFire_Increment(struct Driver *driver, int reserves, u32 type, int fireLevel);
```

`reserves` is the amount granted; `type` is a bitmask
(`include/namespace_Vehicle.h:522-526`):

| Flag | Value | Boost source | Call site |
| --- | --- | --- | --- |
| `POWER_SLIDE_HANG_TIME` | `0x2` | Powerslide and hang-time boosts | `VehPhysProc.c:1884`, `UI_Meter.c:60` |
| `TURBO_PAD` | `0x4` | Turbo pads | `VehPhysForce.c:541-551` |

A single mutable-event seam at the top of this function exposes the grant to mods.

### 8.2 Engine-side seam (added once by an engine developer)

Declare the event in the manifest (§5.4):

```c
/* mod_events.def */
MOD_EVENT(VEHICLE_RESERVES_GRANTED, ModEventVehicleReservesGranted, mutable)
```

The generated context carries the boost type (read-only) and the grant amount
(read/write):

```c
typedef struct ModEventVehicleReservesGranted
{
    ModDriverHandle driver;
    ModBoostType    boostType; /* read-only:  which boost is granting */
    int             amount;    /* read/write: reserves to grant */
} ModEventVehicleReservesGranted;
```

Emit it at the top of the grant function — the only change to game source, behind
the feature flag (§3.6):

```c
/* game/Vehicle/VehFire.c, start of VehFire_Increment */
#ifdef CTR_MODDING
    ModEventVehicleReservesGranted reservesEvent = {
        ModDriver_Wrap(driver), (ModBoostType)type, reserves,
    };
    MOD_EMIT_MUTABLE(EVENT_VEHICLE_RESERVES_GRANTED, &reservesEvent);
    reserves = reservesEvent.amount; /* read mutated value back */
#endif
```

That is the entire engine-side cost: one `.def` line and one emit block. Every
present and future reserve-granting path is now moddable.

### 8.3 The mod source

The mod sees only `mod/mod_api.h` — no game headers, no game symbols. It reads the
boost type through an accessor, doubles the amount when it matches, and writes it
back. The `MOD_BOOST_TYPE_*` values are the generated mirror of the game's own boost
flags (§5.4), static-asserted equal to them — reused, not reinvented:

```c
#include "mod/mod_api.h"

static const ModApi *gApi;

static ModEventVerdict OnReservesGranted(ModEventContextHandle context, void *userData)
{
    (void)userData;

    const ModBoostType doubledSources =
        MOD_BOOST_TYPE_POWER_SLIDE_HANG_TIME | MOD_BOOST_TYPE_TURBO_PAD;

    if ((gApi->reservesGrantedGetBoostType(context) & doubledSources) == 0)
    {
        return MOD_EVENT_VERDICT_CONTINUE; /* not a boost we care about */
    }

    const int amount = gApi->reservesGrantedGetAmount(context);
    gApi->reservesGrantedSetAmount(context, amount * 2);

    return MOD_EVENT_VERDICT_CONTINUE;
}

MOD_EXPORT int ModRegister(const ModApi *api, ModContextHandle self)
{
    gApi = api;
    api->eventSubscribe(self, MOD_EVENT_VEHICLE_RESERVES_GRANTED, OnReservesGranted, NULL);
    api->logMessage(self, MOD_LOG_LEVEL_INFO, "Double Reserves active.");
    return 0;
}
```

The callback is small, side-effect-free except through the injected context, and
trivially unit-testable with a fake context (principle 4).

### 8.4 The package

```
double-reserves.zip
  package.json
  double_reserves.dll
```

```jsonc
{
  "id": "com.example.double-reserves",
  "name": "Double Reserves",
  "version": "1.0.0",
  "apiVersion": "1.0",
  "author": "Example Author",
  "description": "Powerslide, turbo-pad, and hangtime boosts grant double reserves.",
  "entryPoint": "double_reserves.dll",

  "capabilities": {
    "events": [
      { "id": "vehicle.reservesGranted", "mode": "mutable" }
    ]
  }
}
```

### 8.5 What the loader infers

- **Compatibility class: `OrderSensitive`** (§7.4) — the mod uses a *mutable* event.
  Two reserve-doubling mods would stack multiplicatively in load order rather than
  conflict, so they combine without a hard error.
- **Simulation-affecting: yes** (§9) — reserves drive physics, so the mod's
  `(id, version)` is hashed into replay/savestate metadata; a ghost recorded with it
  active is flagged if replayed without it.
- **Manifest enforcement** (§7.3) — the mod declared only the `vehicle.reservesGranted`
  mutable subscription. If its code tried to install a hook or subscribe to anything
  else, registration would be rejected.

This is the payoff of the architecture: a gameplay rule that would otherwise require
editing and recompiling `VehFire.c` becomes a ~20-line, redistributable, composable
package — and the same seam is reachable from a script in the Phase 4 tier with no
further engine work.

## 9. Determinism, replays, and savestates

The game records ghost replays (`game/GhostReplay.c`) and savestates
(`platform/native_savestate.c`). Mods that alter simulation desynchronize these.

- A mod is **simulation-affecting** if it installs hooks on simulation functions or
  subscribes to mutable/veto events on simulation seams. `affectsSimulation` may be
  declared, but the loader also derives it from capabilities and uses the stricter
  of the two.
- The loader maintains the set of active simulation-affecting mods and hashes their
  `(id, version)` pairs into replay and savestate metadata. On load, a mismatch
  between the recording's mod-set and the active mod-set is surfaced as a warning,
  so desynced replays are diagnosable rather than silently wrong.