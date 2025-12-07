# Readme
- any changes to the source files in patches must be made as a patch file
- switch to the patch-netcode branch, rebase it onto master, make your change in a new commit and generate a patch file with ``````
- commit the patch file to master in the patches folder
- imperfect solution for now:
    - add ```git apply engine/src/vendor/patches<yourfile>.patch``` in BuildDebug.bat and BuildRelease.bat, BuildDebug.sh and BuildRelease.sh and then after the build ```git apply -R engine/src/vendor/patches<yourfile>.patch```