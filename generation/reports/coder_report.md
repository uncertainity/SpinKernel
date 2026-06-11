# Coder Report

## Files Generated

- `generation/baseline/core.cpp`
- `generation/tests/test_core.cpp`
- `generation/run_commands.txt`
- `generation/reports/coder_report.md`

## Compilation Status

- Command: `g++ -std=c++17 -O0 -g ./generation/baseline/core.cpp ./generation/tests/test_core.cpp -o ./generation/tests/test_core`
- Status: PASS

## Tests Implemented

- Forced pay-window tests for base no-win oracle, 2OAK ways, WILD substitution, base SCAT trigger, single/two COLLECT behavior, Free Spins WILD multiplier, Free Spins retrigger, Blitz collect total, additional COR eligibility, forced reel stops, and trace shape checks.
- Tests directly call contract functions where possible: `generatePayWindow`, `additionalCOR`, `waysWinCalculation`, `waysWinCalculationWithMultiplier`, and `resolveCollect`.
- Test helpers support forced windows, forced COR xBet values, forced WILD multipliers, forced weighted-table selections, and forced reel stops.

## Test Results

- Command: `./generation/tests/test_core`
- Status: FAIL due to oracle/contract blockers recorded in `validation/reports/escalation.md`.
- Passing assertions cover scatter counts/wins, collect/COR behavior, feature award mapping, additional COR placement restrictions, forced stop window generation, and trace shape emission.
- Failing assertions:
  - `TC-BASE-001 ways_win`: oracle expects `0`, but the forced window contains contract-paying left-to-right standard-symbol matches.
  - `TC-BASE-003 ways_win`: oracle expects only the narrated HV1 WILD-substitution win, while the contract evaluates all standard symbols with WILD substitution.
  - `TC-FS-001 multiplied ways_win`: same all-standard-symbol WILD-substitution conflict after multiplier application.

## Trace Instrumentation Status

- PASS for all Coder contract functions. Each contract function instantiates `Trace` at function entry and emits required inputs, outputs, and Global/Struct dependencies using the `trace.cpp` formatting style.
- Contract functions instrumented: `generatePayWindow`, `additionalCOR`, `waysWinCalculation`, `waysWinCalculationWithMultiplier`, `resolveCollect`, `FreeGameFeature_1`, `FreeGameFeature_2`, `runOneSpin`.

## Warnings And Ambiguities

- Free Spins SCAT evaluation remains configurable through `freeSpinsScatterEvaluationRule`; default is `ANYWHERE` because the Coder contract requires the ambiguity to remain explicit.
- The 4096-ways duplicate-counting ambiguity remains active. Implementation follows the Coder contract pseudocode and evaluates every standard symbol.
- Buy Bonus is not implemented because this Coder-stage request explicitly said not to generate demo code, tune RTP, optimize, or invent mechanics; the contract functions do not include a Buy Bonus entry point.

## Blockers

- `[BLOCKER]` diagnostics were appended to `validation/reports/escalation.md` for deterministic oracle cases that conflict with `coder_contract.md` ways evaluation.
- The baseline implementation preserves `coder_contract.md`; no game mathematics were changed to make conflicting oracle assertions pass.
