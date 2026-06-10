#!/usr/bin/env bash
set -euo pipefail

SKILLS="./skills"

declare -A SKILL_DESC=(
  ["planner"]="Read the game rule files and math model files, then produce a complete game-flow plan. The planner should describe the lifecycle of a spin: bet setup, reel/window generation, symbol interpretation, base-game evaluation, feature triggers, free-spin or bonus transitions, win accumulation, and final payout. It should focus on understanding what the game does, not writing code."

  ["tester"]="Validate that the extracted game specification contains all information required to build and test a simulator. The tester should check for missing paytables, reel strips, paylines/ways rules, symbol definitions, wild/scatter behavior, feature triggers, free-spin rules, bet multipliers, RTP-related parameters, and test-case expectations. It should produce compact syntax-error-style diagnostics.
  Information about missing inputs should be written in ./validation/reports/escalation.md. The test case expectation file should be inside validation/oracle/test_cases.txt."

  ["simulator"]="Take the validated game-flow plan and convert it into simulator-level pseudocode or an implementation schema. It should define functions such as generatePayWindow(), evaluateBaseGame(), evaluateLines(), evaluateWays(), triggerFeature(), runFreeSpins(), accumulateWins(), and runSimulation(). The output should describe data structures, function inputs/outputs, and control flow.
  The implementation schema should be written under ./generation/baseline/implementation_schema.json and it should follow a call tree structure with each function describing what it focuses on in psuedocode."

  ["coder"]="Convert the simulator schema into working code. The coder should write clean baseline code first, compile or run it, test it against provided Q&A/test cases, then optionally produce an optimized version. It should not change game mathematics unless explicitly instructed.
  The clean baseline code should be kept in ./baseline/core.cpp. The optimized code should be in ./optimize/efficient_core.cpp. A playable version of the game in C++ is also required. It should toggle between spin by spin mode and simulation mode. This needs to be written under 
  ./demo/game.cpp. Also write the terminal commands required to run the generated codes for all cases in ./generation/run_commands.txt"

  ["mathematician"]="Analyze and tune the math model. Given target RTP, hit rate, volatility, feature frequency, or win distribution constraints, this skill should suggest parameter changes using controlled search, Markov grammar edits, hyperparameter tuning, or simulation-based evaluation. It should preserve game-rule validity while improving mathematical targets. You have the power to copy the ./generation/optimizer code and 
  build a suitable workflow inside the folder ./math and match the required targets."
)

mkdir -p "$SKILLS"

for skill in "${!SKILL_DESC[@]}"; do
    SKILL_DIR="${SKILLS}/${skill}"
    TASK_FILE="${SKILL_DIR}/TASK.md"
    SCHEMA_FILE="${SKILL_DIR}/${skill}_ir.schema.json"

    mkdir -p "$SKILL_DIR"

    cat > "$TASK_FILE" <<EOF
# ${skill}

## Role

${SKILL_DESC[$skill]}

## Inputs

- gameRule files
- mathModel files
- test case files
- reflib / existing simulator reference, if available

## Output

Describe expected output contract here.

## Failure Mode

If required information is missing, report it clearly and minimally.
EOF

    cat > "$SCHEMA_FILE" <<EOF
{
  "\$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "${skill} IR",
  "type": "object",
  "properties": {},
  "required": []
}
EOF

    echo "[CREATED] $SKILL_DIR"
    echo "[CREATED] $TASK_FILE"
    echo "[CREATED] $SCHEMA_FILE"
done