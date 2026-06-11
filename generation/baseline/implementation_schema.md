# Vegas Blitz Implementation Schema

JSON authority: `generation/baseline/implementation_schema.json`.

## Scope

This is the Simulator-stage architecture for Vegas Blitz. It defines data contracts, state, function contracts, call graph, event flow, execution traces, and oracle mapping only. It does not write C++ and does not change game mathematics.

## Trace Verification

Status: corrected with warnings and one unresolved ambiguity. The corrected trace is written to `generation/simulator/corrected_trace_ref.txt`.

Key corrections:

- `SCATTER` is a trace alias for canonical `SCAT`.
- `LV6` is included in the standard symbol set.
- Trace constants `NO_OF_SYMBOLS = 8` and `SYMBOL_COUNT = 8` are not valid for Vegas Blitz; schema uses `STANDARD_SYMBOL_COUNT = 11` and `PAYTABLE_SYMBOL_COUNT = 12`.
- Reel size vectors are shape `[6]`, not `[5]`.
- PayWindow storage is row-major `[4x6]`, matching four visible rows by six reels.
- The trace paytable shape `[9x6]` is replaced by a normalized symbol-keyed paytable with matrix view `[12x7]` for count indexes 0..6.
- Base Collect resolution is inserted after ways/scatter evaluation and before feature handling.
- `FreeGameFeature_2` is retained as a trace alias for Blitz Spins.

## Symbols And Constants

Canonical symbols: HV1, HV2, HV3, HV4, HV5, LV1, LV2, LV3, LV4, LV5, LV6, WILD, SCAT, COR, COLLECT.

Core constants:

- `NO_OF_REELS = 6`
- `NO_OF_ROWS = 4`
- `PAY_WINDOW_SHAPE = [4x6]`
- `BASE_BET_FIXED_COINS = 20`
- `STANDARD_SYMBOL_COUNT = 11`
- `PAYTABLE_SYMBOL_COUNT = 12`

## Reelsets

Required reelsets are present in the JSON schema with full strips and sizes:

- `BG1_Reels` from `reel_strips.base_B1`
- `BG2_Reels` from `reel_strips.base_B2`
- `FG1_Reels` from `reel_strips.free_F1`
- Buy-bonus Free Spins reelsets `BB_F2_for_4_scatters`, `BB_F3_for_5_scatters`, `BB_F4_for_6_scatters`
- Buy-bonus Blitz pseudo-trigger reelsets where provided by Planner

## Probability Tables

The schema maps trace table names to planner tables:

- `probTable.ReelsetWeights` -> base reelset weights B1=58, B2=42
- `probTable.corTrigger` -> base additional COR trigger Yes=1, No=9
- `probTable.numCorWeight` -> additional COR count weights 1..5
- `probTable.wildMult` -> Free Spins WILD multiplier weights 2x/3x/5x
- `probTable.freeGamenumCollect` -> Blitz collect count weights
- `probTable.freeGameCOROccur` -> Blitz COR occurrence weights
- `probTable.goodCORTable`, `badCORTable`, `CORValues` -> Blitz COR value tables

## Core Data Structures

The Coder should implement these structures from the JSON contract:

- `Symbol`
- `ReelSet`
- `PayWindow`
- `WinBreakdown`
- `CollectResult`
- `FeatureState`
- `GameState`
- `SpinResult`

## Function Contracts

Required functions:

- `runOneSpin`
- `generatePayWindow`
- `additionalCOR`
- `waysWinCalculation`
- `waysWinCalculationWithMultiplier`
- `resolveCollect`
- `FreeGameFeature_1`
- `FreeGameFeature_2`

Each function's inputs, outputs, dependencies, shapes, and pseudocode are defined in the authoritative JSON.

## Call Graph

No feature path:

`runOneSpin -> generatePayWindow -> additionalCOR -> waysWinCalculation -> resolveCollect -> return SpinResult`

Free Spins path:

`runOneSpin -> base path -> FreeGameFeature_1 -> generatePayWindow -> waysWinCalculationWithMultiplier -> return SpinResult`

Blitz Spins path:

`runOneSpin -> base path -> FreeGameFeature_2 -> resolveCollect per Blitz spin -> return SpinResult`

## Feature Architecture

Free Spins use `FG1_Reels` for natural triggers, WILD multipliers sampled from `probTable.wildMult`, and retriggers for 2/3/4/5/6 SCAT. COR and COLLECT do not appear.

Blitz Spins generate only COR, jackpot-valued COR, and COLLECT symbols. Each spin guarantees at least one COLLECT and at least one COR, uses one Good-table COR value and Bad-table values for all other CORs, and has no retriggers.

Collect is active only in base game and Blitz Spins. Each COLLECT independently collects all visible COR and jackpot values.

## Oracle Mapping

Oracle mappings are included in JSON under `oracle_test_mapping`, covering base no-win, ways wins, WILD substitution, SCAT trigger, Collect, Free Spins multiplier/retrigger, Blitz collection, Buy Bonus config, and additional COR placement.

## Unresolved Items

The schema preserves these unresolved issues without inventing mechanics:

- Free Spins SCAT evaluation: anywhere versus left-to-right.
- Formal duplicate-counting interpretation for 4096 ways wording.
- Buy Bonus trigger display count conflict.
- Coin denomination and complete bet ladder.
