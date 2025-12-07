#!/bin/bash
# TEMPORARY SOLUTION TODO: MOVE TO CMAKE
git apply engine/src/vendor/patches/0001-PRIx64-macro-manually-expanded.patch
git apply engine/src/vendor/patches/0001-exposed-struct-netcode_network_simulator_t.patch


bash Build_Internal.sh Debug
./build/enginetest/StardewEngineTest


# TEMPORARY SOLUTION TODO: MOVE TO CMAKE
git apply -R engine/src/vendor/patches/0001-exposed-struct-netcode_network_simulator_t.patch
git apply -R engine/src/vendor/patches/0001-PRIx64-macro-manually-expanded.patch
