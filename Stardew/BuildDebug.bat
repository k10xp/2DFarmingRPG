

rem "TEMPORARY SOLUTION TODO: MOVE TO CMAKE"
git apply engine/src/vendor/patches/0001-PRIx64-macro-manually-expanded.patch
git apply engine/src/vendor/patches/0001-exposed-struct-netcode_network_simulator_t.patch

if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=generators\conan_toolchain.cmake  -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
cmake --build . --config Debug
rem For debugging with visual studio they need to go here
robocopy "..\Assets" "game\Assets" /E /XO
robocopy "..\Assets" "atlastool\Assets" /E /XO
robocopy "engine\src\Debug" "game\Debug" "StardewEngine.dll" /E /XO
robocopy "engine\src\Debug" "enginetest\Debug" "StardewEngine.dll" /E /XO
robocopy "engine\src\Debug" "atlastool\Debug" "StardewEngine.dll" /E /XO
robocopy "..\enginetest\data" "enginetest\Debug\data" /E /XO

rem "Debugging in visual studio has the working directory path relative to the project folder by default"
robocopy "..\enginetest\data" "enginetest\Debug" /E /XO
cd enginetest\Debug
StardewEngineTest.exe

rem "TEMPORARY SOLUTION TODO: MOVE TO CMAKE"
git apply -R engine/src/vendor/patches/0001-exposed-struct-netcode_network_simulator_t.patch
git apply -R engine/src/vendor/patches/0001-PRIx64-macro-manually-expanded.patch

pause