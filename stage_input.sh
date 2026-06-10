#!/usr/bin/env bash
set -euo pipefail


INPUT="./input"
VALIDATION="./validation"
GAME_DIR=""
RULE_FILES=()
MATH_MODELS=()
TEST_CASES=()

while [[ $# -gt 0 ]]; do
    case "$1" in 
    --game-dir)
        GAME_DIR="$2"
        shift 2
        ;;
    --rule-files)
        shift
        while [[ $# -gt 0 && "$1" != --* ]]; do
            RULE_FILES+=("$1")
            shift
        done
        ;;
    --math-models)
        shift
        while [[ $# -gt 0 && "$1" != --* ]]; do
            MATH_MODELS+=("$1")
            shift
        done
        ;;
    --test-cases)
        shift
        while [[ $# -gt 0 && "$1" != --* ]];do
            TEST_CASES+=("$1")
            shift
        done
        ;;
    *)
        echo "Unknown argument: $1"
        exit 1
        ;;
    esac
done

echo "GAME_DIR  = $GAME_DIR"
echo "Rule files:"
for file in "${RULE_FILES[@]}"; do
    cp "${GAME_DIR}/${file}" "${INPUT}/gameRule"
    echo "${file} is moved in ${INPUT}/gameRule"
done    

echo "Math models:"
for file in "${MATH_MODELS[@]}"; do
    cp "${GAME_DIR}/${file}" "${INPUT}/mathModel"
    echo "${file} is moved in ${INPUT}/mathModel"
done

echo "Test Cases:"
for file in "${TEST_CASES[@]}"; do
    cp "${GAME_DIR}/${file}" "${VALIDATION}/testCases"
    echo "${file} is moved in ${VALIDATION}/testCases"
done

