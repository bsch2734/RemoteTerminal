#run this once to set up the battleship service

#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   APP_ROOT=/opt/battleship INSTALL_DIR=/opt/battleship/install SERVICE_NAME=battleship bash scripts/setup_systemd.sh
#
# Defaults are chosen to match the recommended layout.

SERVICE_NAME="${SERVICE_NAME:-battleship}"
APP_ROOT="${APP_ROOT:-/opt/battleship}"
INSTALL_DIR="${INSTALL_DIR:-${APP_ROOT}/install}"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UNIT_TEMPLATE="${REPO_ROOT}/infra/systemd/battleship.service"
UNIT_DST="/etc/systemd/system/${SERVICE_NAME}.service"

if [[ ! -f "${UNIT_TEMPLATE}" ]]; then
  echo "ERROR: systemd unit template not found: ${UNIT_TEMPLATE}" >&2
  exit 1
fi

# Basic sanity checks before we write the unit:
if [[ ! -d "${APP_ROOT}" ]]; then
  echo "ERROR: APP_ROOT does not exist: ${APP_ROOT}" >&2
  echo "Hint: create it (e.g. sudo mkdir -p ${APP_ROOT}) and ensure permissions are correct." >&2
  exit 1
fi

if [[ ! -d "${INSTALL_DIR}" ]]; then
  echo "WARNING: INSTALL_DIR does not exist yet: ${INSTALL_DIR}" >&2
  echo "That's OK if you haven't run 'make deploy' yet. The service may fail to start until installed." >&2
fi

# Ensure sudo exists (we need it)
if ! command -v sudo >/dev/null 2>&1; then
  echo "ERROR: missing 'sudo'. On Ubuntu: sudo apt install -y sudo" >&2
  exit 1
fi

tmp="$(mktemp)"
cleanup() { rm -f "${tmp}"; }
trap cleanup EXIT

# Replace placeholders in the unit template:
#   __APP_ROOT__    -> APP_ROOT
#   __INSTALL_DIR__ -> INSTALL_DIR
#   __SERVICE_NAME__-> SERVICE_NAME (optional placeholder)
sed \
  -e "s|__APP_ROOT__|${APP_ROOT}|g" \
  -e "s|__INSTALL_DIR__|${INSTALL_DIR}|g" \
  -e "s|__SERVICE_NAME__|${SERVICE_NAME}|g" \
  "${UNIT_TEMPLATE}" > "${tmp}"

echo "Installing systemd unit:"
echo "  Service name : ${SERVICE_NAME}"
echo "  App root     : ${APP_ROOT}"
echo "  Install dir  : ${INSTALL_DIR}"
echo "  Unit file    : ${UNIT_DST}"

sudo cp "${tmp}" "${UNIT_DST}"

sudo systemctl daemon-reload
sudo systemctl enable --now "${SERVICE_NAME}"

echo
echo "Done."
echo "Status: sudo systemctl status ${SERVICE_NAME}"
echo "Logs  : sudo journalctl -u ${SERVICE_NAME} -f"
