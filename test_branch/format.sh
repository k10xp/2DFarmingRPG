#!/bin/bash
npx prettier --write .
find . -type d -name "__pycache__" | xargs rm -rf
find . -type d -name "node_modules" | xargs rm -rf