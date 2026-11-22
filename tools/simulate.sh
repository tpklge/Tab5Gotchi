#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
if [ -d /Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk ]; then
 export SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk
fi
mkdir -p build-host
c++ -std=c++17 -I main main/core/engine.cpp tools/simulate.cpp -o build-host/simulate
exec build-host/simulate "${1:-42}"
