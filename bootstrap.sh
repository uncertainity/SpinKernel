#!/usr/bin/env bash
set -u pipefail

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

FOLDERS=(
    "$INPUT"
    "$MATH"
    "$REFLIB"
    "$GENERATION"
    "$VALIDATION"
    "$GAMERULE_FOLDER"
    "$MATHMODEL_FOLDER"
    "$CPP_FOLDER"
    "$NUMBA_FOLDER"
    "$BASELINE_FOLDER"
    "$OPTIMIZED_FOLDER"
    "$DEMO_FOLDER"
    "$TESTCASES_FOLDER"
    "$ORACLE_FOLDER"
    "$REPORTS_FOLDER"
)

for folder in "${FOLDERS[@]}"; do
    mkdir -p "$folder"
    echo "[CREATED] $folder"
done





