#!/bin/bash

if [[ "$1" == "" ]]; then
    echo "Usage: $0 /path/to/dist_files"
    exit 1
fi
DISTROOT=`cd $1; pwd`

macdeployqt $DISTROOT/CCEdit.app
macdeployqt $DISTROOT/CCPlay.app
macdeployqt $DISTROOT/CCHack.app

# Symlink the other packages to save space
#cd $DISTROOT/CCPlay.app
#rm -r Contents/Frameworks Contents/PlugIns
#ln -s ../CCEdit.app/Contents/Frameworks Contents/Frameworks
#ln -s ../CCEdit.app/Contents/PlugIns Contents/PlugIns

# Move everything to the right places for the packager
rm -rf $DISTROOT/Package_Root
mkdir -p $DISTROOT/Package_Root/Applications
mkdir -p $DISTROOT/Package_Root/usr
mv $DISTROOT/CCEdit.app $DISTROOT/Package_Root/Applications/
mv $DISTROOT/CCPlay.app $DISTROOT/Package_Root/Applications/
mv $DISTROOT/CCHack.app $DISTROOT/Package_Root/Applications/
mv $DISTROOT/share $DISTROOT/Package_Root/usr/
