#!/bin/bash
cd Stardew
sh ./GetDependenciesConan.sh
sh ./BuildDebug.sh
sh ./compile_assets.sh