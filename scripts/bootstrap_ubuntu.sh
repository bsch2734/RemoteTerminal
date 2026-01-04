#run this to automatially install all required prerequisites

#!/usr/bin/env bash
set -euo pipefail

sudo apt update

# Core build + tooling
sudo apt install -y \
  git \
  cmake \
  ninja-build \
  build-essential \
  pkg-config \
  ca-certificates

# Useful for debugging/verification
sudo apt install -y \
  curl \
  openssl
