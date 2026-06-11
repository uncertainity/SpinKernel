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
}

simulator_human_gate(){
    simulator
    read -rp "simulator has finished. Waiting for human verification. Update/Check ./agentprompts/run_sim_human_gate.txt and
press Enter to proceed to the next stage.
"
    run_codex \
    "simulator_human_gate" \
    "./agentprompts/run_sim_human_gate.txt"
    echo "Simulator-Human Gate has been completed."

}

coder(){
    run_codex \
    "coder" \
    "./agentprompts/run_coder.txt"
}

interactive_pass() {
    while grep -q "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"; do

        run_codex \
            "interactive pass" \
            "./agentprompts/interactive_pass.txt"

        grep -n "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"

        if grep -q "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"; then
            read -rp "
[BLOCKER] still exists.
Edit escalation.md or interactive_pass.txt.
Press Enter when ready...
"
        fi
    done
    echo "All blockers are removed."
}

test_augmenter(){
    run_codex \
    "test_augmenter" \
    "./agentprompts/run_test_augmenter.txt"
    if grep -q "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"; then
        echo "[BLOCKER] found in escalation.md."
    fi
}

consensus_loop() {
    local max_attempts=5
    local attempt=1

    while (( attempt <= max_attempts )); do
        echo "Consensus attempt ${attempt}/${max_attempts}"

        test_augmenter

        if ! grep -q "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"; then
            echo "Consensus loop passed."
            log "No of attempts in Concensus Loop = ${attempt}"
            return 0
        fi

        echo "[BLOCKER] found after test_augmenter."
        echo "Running simulator to resolve contract/schema issue."

        run_codex \
            "simulator consensus repair" \
            "./agentprompts/run_simulator_consensus_repair.txt"

        run_codex \
            "coder consensus repair" \
            "./agentprompts/run_coder.txt"

        ((attempt++))
    done
    echo "Consensus loop failed after ${max_attempts} attempts."
    echo "Starting interactive pass."
    interactive_pass
}



usage() {
    cat <<EOF
Usage: $0 <command>

Commands:
  full                Run full SlotCore pipeline
  planner             Run planner only
  tester              Run tester only
  planner-tester      Run planner <-> tester loop
  simulator           Run simulator only
  simulator-gate      Run simulator + human gate
  coder               Run coder only
  interactive-pass    Run human interactive blocker pass
  test-augmenter      Run test augmenter only
  consensus           Run simulator -> coder -> test_augmenter consensus loop
EOF
}

main_pipeline() {
    planner_tester_loop

    simulator_human_gate

    coder

    if grep -q "\[BLOCKER\]" "${REPORTS_FOLDER}/escalation.md"; then
        interactive_pass
    fi

    consensus_loop

    log "Full SlotCore pipeline completed successfully."
}

cmd="${1:-}"

case "$cmd" in
    full)
        main_pipeline
        ;;

    planner)
        planner
        ;;

    tester)
        tester
        ;;

    planner-tester)
        planner_tester_loop
        ;;

    simulator)
        simulator
        ;;

    simulator-gate)
        simulator_human_gate
        ;;

    coder)
        coder
        ;;

    interactive-pass)
        interactive_pass
        ;;

    test-augmenter)
        test_augmenter
        ;;

    consensus)
        consensus_loop
        ;;

    ""|-h|--help|help)
        usage
        ;;

    *)
        echo "Unknown command: $cmd"
        usage
        exit 1
        ;;
esac