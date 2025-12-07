# Readme
- any changes to the source files in vendor must be made as a patch file using this method:
- switch to the patch-netcode branch, rebase it onto master, make your change in a new commit and generate a patch file with ```git format-patch```. Make it use the previous patch as a base so each patch is one commit
- commit the patch file to master in the patches folder
- imperfect solution for now:
    - add ```git apply engine/src/vendor/patches<yourfile>.patch``` in BuildDebug.bat and BuildRelease.bat, BuildDebug.sh and BuildRelease.sh and then after the build ```git apply -R engine/src/vendor/patches<yourfile>.patch```

# Patches
## 0001-PRIx64-macro-manually-expanded.patch
- Without this the windows MSVC build will not compile, manually expands PRIx64 macro to "lx"