#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="${ROOT_DIR}/logs"

mkdir -p "$LOG_DIR"

TS=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="${LOG_DIR}/pipeline_${TS}.log"

log(){
    echo "[$(date '+%F %T')] $*" | tee -a "$LOG_FILE"
}

run_codex() {
    local STAGE="$1"
    local PROMPT="$2"

    log "Running stage: ${STAGE}"
    codex exec "$(cat "$PROMPT")" 2>&1 | tee -a "$LOG_FILE"
    log "Finished stage: ${STAGE}"
}


check_file() {
    local FILE="$1"

    if [[ ! -s "$FILE" ]]; then
        log "ERROR: Missing required artifact: $FILE"
        exit 1
    fi
}

###############################################################################
# Input Validation
###############################################################################

log "Running validate_inputs.sh"

bash ./validate_inputs.sh \
    2>&1 | tee -a "$LOG_FILE"

log "Input validation passed"

###############################################################################
# Planner
###############################################################################

run_codex \
    "planner" \
    "./agentprompts/run_planner.txt"

check_file "./generation/plan/game_flow.md"
check_file "./generation/plan/game_flow.json"

###############################################################################
# Tester
###############################################################################

run_codex \
    "tester" \
    "./prompts/run_tester.txt"

check_file "./validation/reports/validation_report.md"
check_file "./validation/reports/escalation.md"
check_file "./validation/oracle/test_cases.txt"

###############################################################################

log "Planner + Tester pipeline completed successfully"

