
python engine/scripts/ConvertTiled.py ./Assets/out -m ./Assets/Farm.json ./Assets/House.json ./Assets/RoadToTown.json -a engine/scripts/AtlasTool.exe -bmp Atlas.bmp -iw 128 -ih 128

python ./engine/scripts/ExpandAnimations.py -o ./Assets/out/expanded_named_sprites.xml ./Assets/out/named_sprites.xml

python engine/scripts/MergeAtlases.py ./Assets/out/atlas.xml ./Assets/out/expanded_named_sprites.xml > ./Assets/out/atlascombined.xml

"engine/scripts/AtlasTool.exe" ./Assets/out/atlascombined.xml -o ./Assets/out/main.atlas

"engine/scripts/AtlasTool.exe" ./Assets/ui_atlas.xml -o ./Assets/ui_atlas.atlas

robocopy "./Assets" "game\Release\Assets" /E /XO

pause