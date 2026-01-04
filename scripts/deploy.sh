#!/usr/bin/env bash
set -euo pipefail

# ---- prerequisites ----------------------------------------------------------

require_cmd() {
  local cmd="$1"
  local ubuntu_pkg="${2:-$1}"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    echo "ERROR: missing '${cmd}'. On Ubuntu: sudo apt install -y ${ubuntu_pkg}" >&2
    exit 1
  fi
}

require_cmd git git
require_cmd cmake cmake
require_cmd ninja ninja-build

# A C++ compiler (either g++ or clang++)
if command -v g++ >/dev/null 2>&1; then
  : # ok
elif command -v clang++ >/dev/null 2>&1; then
  : # ok
else
  echo "ERROR: missing C++ compiler. On Ubuntu: sudo apt install -y build-essential (or clang)" >&2
  exit 1
fi

# Optional but commonly needed by builds
require_cmd pkg-config pkg-config
# ---- end of prerequisites ----------------------------------------------------------


# optional local config
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -f "${SCRIPT_DIR}/deploy.env" ]]; then
  # shellcheck disable=SC1091
  source "${SCRIPT_DIR}/deploy.env"
fi

: "${APP_ROOT:=/opt/battleship}"
: "${SRC_DIR:=${APP_ROOT}/src}"
: "${BUILD_DIR:=${APP_ROOT}/build}"
: "${INSTALL_DIR:=${APP_ROOT}/install}"
: "${BUILD_TYPE:=Release}"
: "${SERVICE_NAME:=battleship}"

cd "${SRC_DIR}"
git pull --rebase
git submodule update --init --recursive

cmake -S . -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"

cmake --build "${BUILD_DIR}" -j
cmake --install "${BUILD_DIR}"

sudo systemctl restart "${SERVICE_NAME}"
sudo systemctl --no-pager --full status "${SERVICE_NAME}" | cat
