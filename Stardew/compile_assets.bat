
python engine/scripts/ConvertTiled.py ./WfAssets/out -m ./WfAssets/Farm.json ./WfAssets/House.json ./WfAssets/RoadToTown.json -a engine/scripts/AtlasTool.exe -bmp Atlas.bmp -iw 128 -ih 128

python ./engine/scripts/ExpandAnimations.py -o ./WfAssets/out/expanded_named_sprites.xml ./WfAssets/out/named_sprites.xml

python engine/scripts/MergeAtlases.py ./WfAssets/out/atlas.xml ./WfAssets/out/expanded_named_sprites.xml > ./WfAssets/out/atlascombined.xml

"engine/scripts/AtlasTool.exe" ./WfAssets/out/atlascombined.xml -o ./WfAssets/out/main.atlas

"engine/scripts/AtlasTool.exe" ./WfAssets/ui_atlas.xml -o ./WfAssets/ui_atlas.atlas

"game/Release/WarFarmer.exe" --outPersistantFile ./WfAssets/Saves/Dev/Persistant.game

robocopy "./WfAssets/" "game\Release/WfAssets/" /E *.tilemap
robocopy "./WfAssets/" "game\Release/WfAssets/" /E *.xml
robocopy "./WfAssets/" "game\Release/WfAssets/" /E *.lua
robocopy "./WfAssets/" "game\Release/WfAssets/" /E *.atlas
robocopy "./WfAssets/" "game\Release/WfAssets/" /E *.txt
robocopy "./WfAssets/" "game\Release/WfAssets/" /E *.game
robocopy "./WfAssets/ImageFiles.json" "game\Release/WfAssets/ImageFiles.json" /E *.game
robocopy "./WfAssets/Keymap.json" "game\Release/WfAssets/Keymap.json" /E *.game


pause