#!/bin/bash
# TEMPORARY SOLUTION TODO: MOVE TO CMAKE
git apply engine/src/vendor/patches/0001-PRIx64-macro-manually-expanded.patch


bash Build_Internal.sh Debug
./build/enginetest/StardewEngineTest


# TEMPORARY SOLUTION TODO: MOVE TO CMAKE
git apply -R engine/src/vendor/patches/0001-PRIx64-macro-manually-expanded.patch
