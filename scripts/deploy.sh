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
require_cmd node nodejs
require_cmd npm npm

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

# Paths (override by exporting before running deploy.sh)
: "${APP_ROOT:=/opt/remoteterminal}"
: "${SRC_DIR:=${APP_ROOT}/src}"
: "${BUILD_DIR:=${APP_ROOT}/build}"
: "${INSTALL_DIR:=${APP_ROOT}/install}"
: "${BUILD_TYPE:=Release}"
: "${SERVICE_NAME:=remoteterminal}"
: "${WEB_ROOT:=${APP_ROOT}/www}"

cd "${SRC_DIR}"
git pull --rebase
git submodule update --init --recursive

cmake -S . -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"

cmake --build "${BUILD_DIR}" -j
cmake --install "${BUILD_DIR}"

# ---- Deploy webapps (Vite) ----
# Each webapp: [source_dir, dest_subpath]
# dest_subpath is relative to WEB_ROOT ("" means root)
declare -a WEBAPPS=(
  "platform/remoteTerminalWebapp:"
  "games/Battleship/battleshipWebapp:navalbattle"
  "games/TicTacToe/ticTacToeWebapp:tictactoe"
)

# Prepare web root
sudo mkdir -p "${WEB_ROOT}"
sudo rm -rf "${WEB_ROOT:?}/"*

for entry in "${WEBAPPS[@]}"; do
  webapp_src="${entry%%:*}"
  webapp_dest="${entry#*:}"
  webapp_dir="${SRC_DIR}/${webapp_src}"

  if [ -d "${webapp_dir}" ]; then
    echo "[deploy] Building ${webapp_src}..."

    pushd "${webapp_dir}" >/dev/null
    npm ci
    npm run build
    popd >/dev/null

    if [ -z "${webapp_dest}" ]; then
      dest_path="${WEB_ROOT}"
    else
      dest_path="${WEB_ROOT}/${webapp_dest}"
    fi

    echo "[deploy] Publishing ${webapp_src} to ${dest_path}..."
    sudo mkdir -p "${dest_path}"
    sudo cp -r "${webapp_dir}/dist/"* "${dest_path}/"

    echo "[deploy] ${webapp_src} deployed."
  else
    echo "[deploy] WARNING: ${webapp_dir} not found, skipping."
  fi
done

echo "[deploy] All webapps deployed."


sudo systemctl restart "${SERVICE_NAME}"
sudo systemctl --no-pager --full status "${SERVICE_NAME}" | cat
