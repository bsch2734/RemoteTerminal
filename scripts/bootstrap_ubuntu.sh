#!/usr/bin/env bash
set -euo pipefail

# This script prepares a fresh Ubuntu box to build + deploy the project.

sudo apt-get update

# Core build tools + common deps
sudo apt-get install -y \
  git \
  cmake \
  ninja-build \
  build-essential \
  pkg-config \
  curl \
  ca-certificates

# ---- Node.js (for Vite webapp) ----
# Install Node 20 LTS from NodeSource (Ubuntu's nodejs can be too old for Vite).
if ! command -v node >/dev/null 2>&1; then
  curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
  sudo apt-get install -y nodejs
fi

# Optional: ensure git submodules work via SSH (user handles keys separately)

echo "Bootstrap complete."
