# Slot Agents

## Purpose

This repository uses a staged agent pipeline for converting slot-game documents into a validated simulator, tested C++ implementation, and mathematical tuning workflow.

The pipeline is intentionally manual-stage-driven. Do not automatically execute all stages unless explicitly instructed.

Each stage has:

* A role prompt under `./skills/`
* Required inputs
* Required outputs
* Validation checkpoints

---

## Fresh Session Rule

At the start of every new Codex session, first read:

```text
./summary.md
./AGENTS.md
```

`summary.md` contains the current project status, completed stages, known issues, pending decisions, and human validation notes.

If `summary.md` is missing, create it before making major changes.

---

## Summary File Contract

`summary.md` must be kept up to date after every major stage.

It should contain:

```text
# SlotCore Summary

## Current Stage

## Completed Stages

## Pending Stages

## Important Decisions

## Known Missing Inputs

## Validation Status

## Generated Artifacts

## Human Review Notes

## Next Recommended Action
```

Do not rely only on chat history. A new Codex session must be able to continue from `summary.md`.

---

## Directory Layout

Expected project layout:

```text
.
├── AGENTS.md
├── summary.md
├── skills/
│   ├── planner.md
│   ├── tester.md
│   ├── simulator.md
│   ├── coder.md
│   └── mathematician.md
│
├── input/
│   ├── gameRule/
│   └── mathModel/
│
├── validation/
│   ├── reports/
│   ├── oracle/
│   └── testCases/
│
├── generation/
│   ├── baseline/
│   ├── optimize/
│   └── demo/
│
└── math/
    ├── parameter_sets/
    ├── reports/
    ├── simulations/
    ├── optimize/
    └── selected/
```

---

## Global Rules

These rules apply to every agent.

1. Never invent slot mechanics.
2. Never invent paytables, paylines, reel strips, symbols, feature triggers, or RTP assumptions.
3. If information is missing, report it clearly.
4. Prefer structured artifacts over free-form prose.
5. Preserve game mathematics exactly unless the Mathematician stage is explicitly active.
6. Baseline correctness comes before optimization.
7. Do not skip validation gates.
8. Keep `summary.md` updated.
9. If blocked, write the blocker to `./validation/reports/escalation.md`.
10. A fresh Codex session must be able to continue using only repository files.

---

## Pipeline Order

The default pipeline is:

```text
Planner
→ Tester
→ Simulator
→ Coder
→ Postcoder
→ Mathematician
```

Do not run later stages before earlier stage outputs exist.

Manual review is expected between stages.

---

## Context Budget Rule

Agents must minimize context usage.

For each stage, read only:

1. ./summary.md
2. ./AGENTS.md
3. the stage skill file
4. the required inputs listed for that stage

Do not recursively inspect unrelated directories.

Do not read raw input documents unless the current stage explicitly lists them as inputs or an upstream artifact marks them as unresolved references.

If additional files are needed, first record the reason in the .validation/escalation.md report. You may try to use summary.md 
instead as much as possible.

---

## Stage 1: Planner

### Skill File

Read:

```text
./skills/planner.md
```

### Inputs

```text
./input/gameRule/
./input/mathModel/
```

### Outputs

```text
./generation/plan/game_flow.md
./generation/plan/game_flow.json
```

### Purpose

The Planner understands the slot game specification and produces an implementation-independent game-flow description.

The Planner must describe:

* Game overview
* Symbols
* Bet setup
* Base game flow
* Feature flow
* State transitions
* Win evaluation logic
* Mathematical dependencies
* Assumptions
* Missing information

### Do Not

The Planner must not write code or simulator pseudocode.

### After Completion

Update:

```text
./summary.md
```

---

## Upstream Artifact Authority

For every stage after Planner:

- Raw input documents are background reference.
- Previous stage outputs are the primary contract.
- Validation reports override planner assumptions when conflicts exist.
- If raw documents, planner outputs, and validation reports disagree, do not resolve silently.
  Write the conflict to escalation.md.

---

## Stage 2: Tester

### Skill File

Read:

```text
./skills/tester.md
```

### Inputs

```text
./input/gameRule/
./input/mathModel/
./referrenceLibrary/testCases/
./validation/testCases/
./generation/plan/game_flow.md
./generation/plan/game_flow.json
```

### Outputs

```text
./validation/reports/escalation.md
./validation/reports/validation_report.md
./validation/oracle/test_cases.txt
```

### Purpose

The Tester validates whether the extracted specification is complete enough to build and test a simulator.

The Tester must check for:

* Missing paytables
* Missing reel strips
* Missing paylines or ways rules
* Missing symbol definitions
* Missing wild/scatter behavior
* Missing feature rules
* Missing RTP-critical parameters
* Planner inconsistencies

### Diagnostics Style

Prefer compiler-style diagnostics:

```text
[ERROR] Missing paytable
[ERROR] Missing reel strips
[WARNING] Wild substitution rule ambiguous
```

### After Completion

Update:

```text
./summary.md
```

---

## Stage 3: Simulator

### Skill File

Read:

```text
./skills/simulator.md
```

### Inputs

```text
./generation/plan/game_flow.md
./generation/plan/game_flow.json
./validation/reports/escalation.md
./validation/reports/validation_report.md
./validation/oracle/test_cases.txt
```

### Outputs

```text
./generation/baseline/implementation_schema.md
./generation/baseline/implementation_schema.json
```

### Purpose

The Simulator converts the validated game plan into an implementation-ready architecture.

It must define:

* Data structures
* State objects
* Function contracts
* Call graph
* Event flow
* Feature architecture
* Oracle mapping

### Do Not

The Simulator must not write production C++.

### After Completion

Update:

```text
./summary.md
```

---

## Stage 4: Coder

### Skill File

Read:

```text
./skills/coder.md
```

### Inputs

```text
./generation/baseline/implementation_schema.md
./generation/baseline/implementation_schema.json
./validation/oracle/test_cases.txt
./validation/reports/validation_report.md
```

### Outputs

```text
./generation/baseline/core.cpp
./generation/run_commands.txt
./generation/baseline/test_core.cpp
./generation/reports/coder_report.md
```

### Purpose

The Coder converts the simulator schema into working C++.

It must:

1. Write baseline code.
2. Compile baseline code.
3. Run oracle tests.
4. Fix correctness bugs.
5. Write demo code.
6. Write optimized code only after baseline correctness.
7. Write exact run commands.

### Do Not

The Coder must not change game mathematics to make tests pass.

### After Completion

Update:

```text
./summary.md
```

---

## Stage 5: Test Augmenter

### Skill File

Read:

```text
./skills/post_coder_tester.md
```

### Inputs

```text
./generation/baseline/implementation_schema.md
./generation/baseline/implementation_schema.json

./generation/baseline/core.cpp
./generation/tests/test_core.cpp

./generation/reports/coder_report.md

./validation/reports/validation_report.md
./validation/reports/escalation.md

./validation/oracle/test_cases.txt
```

### Outputs

```text
./validation/reports/post_coder_audit.md
./validation/oracle/edge_test_cases.txt
```

Optional:

```text
./validation/reports/coverage_matrix.md
```

### Purpose

The Post-Code Tester audits the generated implementation after the Coder stage.

It attempts to identify:

* Missing edge cases
* Missing test coverage
* Weak oracle tests
* Untested state transitions
* Untested feature paths
* Schema-code inconsistencies
* Deterministic test gaps

The Post-Code Tester does not modify code.

It generates audit reports and additional deterministic test cases only.

### Required Checks

The Post-Code Tester must verify:

* Function coverage
* Call graph coverage
* State machine coverage
* Feature coverage
* Window-level test coverage
* Oracle test quality
* Schema-code consistency

It should generate additional edge cases where coverage is weak.

### Do Not

The Post-Code Tester must not:

* Modify C++ source files
* Rewrite simulator architecture
* Change game mathematics
* Invent slot mechanics
* Generate UI tests
* Generate frontend tests
* Generate animation or rendering tests
* Modify existing oracle tests

### After Completion

Update:

```text
./summary.md
```
----

## Stage 6: Mathematician

### Skill File

Read:

```text
./skills/mathematician.md
```

### Inputs

```text
./generation/baseline/core.cpp
./generation/optimized/efficient_core.cpp
./generation/demo/game.cpp
./input/mathModel/
./validation/reports/validation_report.md
```

Optional:

```text
./math/targets.json
./math/constraints.json
./math/initial_parameters.json
```

### Outputs

```text
./math/reports/math_report.md
./math/reports/rtp_report.csv
./math/reports/hit_rate_report.csv
./math/selected/parameter_set.json
```

### Purpose

The Mathematician tunes configurable mathematical parameters while preserving game-rule correctness.

It may tune:

* Reel composition
* Symbol weights
* Bonus weights
* Feature frequencies
* Multiplier tables
* Configurable probabilities

It must not change game rules.

### After Completion

Update:

```text
./summary.md
```

---

## Manual Routing Policy

This project uses manual routing by default.

Preferred execution style:

```text
Run Planner
→ Human review
→ Run Tester
→ Human review
→ Run Simulator
→ Human review
→ Run Coder
→ Human review
→ Run Mathematician
```

Do not assume permission to run all stages automatically.

If asked to run a specific stage, only run that stage and update `summary.md`.

---

## Stage Completion Checklist

A stage is complete only if:

* Required outputs exist.
* Outputs are non-empty.
* Missing information is reported.
* `summary.md` is updated.
* The next stage has clear inputs.

---

## Escalation Policy

If blocked, write to:

```text
./validation/reports/escalation.md
```

Use compact diagnostics of three kinds - BLOCKER, WARNING, AMBIGUITY

```text
[BLOCKER] Cannot generate simulator schema: reel strips missing.
[BLOCKER] Cannot implement evaluateLines(): paylines missing.
[WARNING] `trace_ref.txt` base `runOneSpin` call path omits Collect resolution after ways/scatter evaluation. Planner and validation reports require Collect resolution before feature trigger handling. Corrected trace inserts `resolveCollect` after `waysWinCalculation`.
[AMBIGUITY] `trace_ref.txt` and Planner outputs do not resolve the already-recorded Free Spins SCAT evaluation conflict: 

```

---

## New Codex Session Startup Checklist

When starting fresh:

1. Read `./summary.md`.
2. Read `./AGENTS.md`.
3. Identify current stage.
4. Read the corresponding `./skills/<stage>.md`.
5. Verify required inputs exist.
6. Continue only from the current stage.
7. Update `summary.md` before stopping.

---

## Core Principle

`skills/*.md` defines how each agent thinks.

`AGENTS.md` defines how the pipeline is wired.

`summary.md` defines where the project currently stands.

Do not confuse these roles.
