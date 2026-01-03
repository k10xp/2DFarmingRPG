# parse versions from game header
MAJOR_VERSION=$(cat ./game/include/WfVersion.h | grep -P -o "(?<=#define WF_MAJOR_VERSION \")[0-9]+(?=\")")
MINOR_VERSION=$(cat ./game/include/WfVersion.h | grep -P -o "(?<=#define WF_MINOR_VERSION \")[0-9]+(?=\")")
REVISION=$(cat ./game/include/WfVersion.h | grep -P -o "(?<=#define WF_REVISION \")[0-9]+(?=\")")
PACKAGE_NAME="OpenFarmer"
DEB_PKG_NAME=$(printf "%s_%s.%s-%s" $PACKAGE_NAME $MAJOR_VERSION $MINOR_VERSION $REVISION)
VERSION_STRING=$(printf "%s.%s-%s" $MAJOR_VERSION $MINOR_VERSION $REVISION)
EXE_NAME="WarFarmer"

# create folder structure
mkdir -p "./$DEB_PKG_NAME/usr/bin"
mkdir -p "./$DEB_PKG_NAME/usr/lib"
mkdir -p "./$DEB_PKG_NAME/usr/share"
mkdir -p "./$DEB_PKG_NAME/DEBIAN"

# install assets
cp -R ./WfAssets "./$DEB_PKG_NAME/usr/share"

# install game exe
cp "./build/game/$EXE_NAME" "./$DEB_PKG_NAME/usr/bin/$EXE_NAME"

# install engine library
cp "./build/engine/src/libStardewEngine.so" "./$DEB_PKG_NAME/usr/lib/libStardewEngine.so"

# install debian control file
cp "./debian_control.txt" "./$DEB_PKG_NAME/DEBIAN"
mv "./$DEB_PKG_NAME/DEBIAN/debian_control.txt" "./$DEB_PKG_NAME/DEBIAN/control"
sed -i "s/<<VERSION>>/$VERSION_STRING/g" "./$DEB_PKG_NAME/DEBIAN/control"

# install script to start the game
echo "(cd /usr/share/; $EXE_NAME \$@)" >> "./$DEB_PKG_NAME/usr/bin/openfarmer"
chmod +x "./$DEB_PKG_NAME/usr/bin/openfarmer"

# build debian package
dpkg-deb --build "./$DEB_PKG_NAME"

# delete staging folder
rm -rf ./$DEB_PKG_NAME

