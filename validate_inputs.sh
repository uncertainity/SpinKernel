#!/usr/bin/env bash
set -uo pipefail

INPUT="./input"
REFLIB="./referrenceLibrary"
VALIDATION="./validation"

GAMERULE_FOLDER="${INPUT}/gameRule"
MATHMODEL_FOLDER="${INPUT}/mathModel"
TESTCASE_SPECIFICATION="${VALIDATION}/testCases"

CPP_FOLDER="${REFLIB}/cpp"
NUMBA_FOLDER="${REFLIB}/numba"

AGENTS_FILE="./AGENTS.md"
MEMORY_FILE="./MEMORY.md"

check_file() {
    local file="$1"

    if [[ ! -f "$file" ]]; then
        echo "[ERROR] Missing file: $file"
        return 1
    elif [[ ! -s "$file" ]]; then
        echo "[WARNING] File exists but is empty: $file"
        return 0
    else
        echo "[OK] File found: $file"
        return 0
    fi
}

check_required_folder() {
    local folder="$1"

    if [[ ! -d "$folder" ]]; then
        echo "[ERROR] Missing folder: $folder"
        return 1
    elif [[ -z "$(ls -A "$folder")" ]]; then
        echo "[ERROR] Folder is empty: $folder"
        return 1
    else
        echo "[OK] Folder found: $folder"
        return 0
    fi
}

check_optional_folder() {
    local folder="$1"

    if [[ ! -d "$folder" ]]; then
        echo "[WARNING] Optional folder missing: $folder"
        return 1
    elif [[ -z "$(ls -A "$folder")" ]]; then
        echo "[WARNING] Optional folder is empty: $folder"
        return 1
    else
        echo "[OK] Optional folder found: $folder"
        return 0
    fi
}

check_required_folder "$GAMERULE_FOLDER"; err_GAMERULE=$?
check_optional_folder "$MATHMODEL_FOLDER"; err_MATHMODEL=$?
check_optional_folder "$TESTCASE_SPECIFICATION"; err_TESTCASE=$?

check_file "$AGENTS_FILE"; err_AGENTS=$?
check_required_folder "$CPP_FOLDER"; err_CPP=$?
check_required_folder "$NUMBA_FOLDER"; err_NUMBA=$?

err_REF=$(( err_CPP | err_NUMBA ))


set -e

if [[ "$err_GAMERULE" -ne 0 ]]; then
    echo "[FAILED] Required GAMERULE missing. Fix errors before running Codex."
    exit 1
fi

if [[ "$err_AGENTS" -ne 0 ]]; then
    echo "[FAILED] Required AGENTS.md missing. Fix errors before running Codex."
    exit 1
fi


if [[ "$err_CPP" -ne 0 && "$err_NUMBA" -ne 0 ]]; then
    echo "[FAILED] Required REFERENCE LIBRARY missing. Fix errors before running Codex."
    exit 1
fi


if [[ "$err_MATHMODEL" -ne 0 ]]; then
    echo "[WARNING] No Math Model has been provided."
fi

if [[ "$err_TESTCASE" -ne 0 ]]; then
    echo "[WARNING] No Test Case Specification has been provided."
fi

echo
echo "[PASSED] All required inputs are present."