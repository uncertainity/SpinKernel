#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="${ROOT_DIR}/logs"
INPUT="./input"
REFLIB="./referrenceLibrary"
GENERATION="./generation"
VALIDATION="./validation"
MATH="./math"

GAMERULE_FOLDER="${INPUT}/gameRule"
MATHMODEL_FOLDER="${INPUT}/mathModel"

CPP_FOLDER="${REFLIB}/cpp"
NUMBA_FOLDER="${REFLIB}/numba"

BASELINE_FOLDER="${GENERATION}/baseline"
OPTIMIZED_FOLDER="${GENERATION}/optimize"
DEMO_FOLDER="${GENERATION}/demo"

TESTCASES_FOLDER="${VALIDATION}/testCases"
ORACLE_FOLDER="${VALIDATION}/oracle"
REPORTS_FOLDER="${VALIDATION}/reports"


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

planner(){
    run_codex \
    "planner" \
    "./agentprompts/run_planner.txt"

    check_file "${GENERATION}/plan/game_flow.md"
    check_file "${GENERATION}/plan/game_flow.json"
}

tester(){
    run_codex \
    "tester" \
    "./agentprompts/run_tester.txt"

    check_file "${REPORTS_FOLDER}/validation_report.md"
    check_file "${REPORTS_FOLDER}/escalation.md"
    check_file "${ORACLE_FOLDER}/test_cases.txt"
    if grep -q "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"; then
        echo "[BLOCKER] found in escalation.md"
        return 1
    fi
    
    return 0
}

###############################################################################
# Planner
###############################################################################



# run_codex \
#     "planner" \
#     "./agentprompts/run_planner.txt"

# check_file "./generation/plan/game_flow.md"
# check_file "./generation/plan/game_flow.json"

###############################################################################
# Tester
###############################################################################

# run_codex \
#     "tester" \
#     "./agentprompts/run_tester.txt"

# check_file "./validation/reports/validation_report.md"
# check_file "./validation/reports/escalation.md"
# check_file "./validation/oracle/test_cases.txt"

###############################################################################

planner_tester_loop(){
    MAX_RETRIES=3
    attempt=1

    while (( attempt <= MAX_RETRIES )); do 
        echo "[INFO] planner/tester attempt ${attempt}/${MAX_RETRIES}"
        planner
        if tester; then
            echo "[SUCCESS] planner output passed tester"
            break
        fi
        echo "[RETRY] tester found blocker; rerunning planner"
        ((attempt++))
    done 

    if (( attempt > MAX_RETRIES )); then
        echo "[ERROR] planner could not pass tester after ${MAX_RETRIES} attempts"
        exit 1
    fi
    log "No of attempts in Planner-Tester Loop = ${attempt}"
    log "Planner + Tester pipeline completed successfully"
}

simulator(){
    run_codex \
    "simulator" \
    "./agentprompts/run_simulator.txt"
    if grep -q "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"; then
        echo "[BLOCKER] found in escalation.md. Human Verification is further required."
    fi
    return 0
}

simulator_human_gate(){
     run_codex \
    "simulator_human_gate" \
    "./agentprompts/run_sim_human_gate.txt"

}

coder(){
    run_codex \
    "coder" \
    "./agentprompts/run_coder.txt"
}

coder
# simulator_human_gate()
log "coder is complete."

#log "Simulator_Human_Gate completed."
# log "Simulator completed. Waiting for Human verification"


