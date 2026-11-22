#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
if [ -d /Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk ]; then
 export SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk
fi
mkdir -p build-host
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I main main/core/engine.cpp main/storage/store.cpp tests/core_test.cpp -o build-host/core_test
build-host/core_test

c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I main main/core/engine.cpp main/extensions/breeding.cpp tests/breeding_test.cpp -o build-host/breeding_test
build-host/breeding_test
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I main main/core/engine.cpp main/storage/store.cpp main/extensions/visit_protocol.cpp tests/visit_test.cpp -o build-host/visit_test
build-host/visit_test

c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I main main/core/engine.cpp main/storage/store.cpp tests/power_test.cpp -o build-host/power_test
build-host/power_test

c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I main main/core/engine.cpp main/storage/store.cpp tests/progression_test.cpp -o build-host/progression_test
build-host/progression_test

c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I main tests/keyboard_test.cpp -o build-host/keyboard_test
build-host/keyboard_test
