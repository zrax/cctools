#!/bin/bash

if [[ "$1" == "" ]]; then
    echo "Usage: $0 /path/to/dist_files"
    exit 1
fi
cd "$1"

macdeployqt CCEdit.app -executable=CCEdit.app/Contents/MacOS/CCEdit -appstore-compliant
macdeployqt CC2Edit.app -executable=CC2Edit.app/Contents/MacOS/CC2Edit -appstore-compliant
macdeployqt CCPlay.app -executable=CCPlay.app/Contents/MacOS/CCPlay -appstore-compliant
macdeployqt CCHack.app -executable=CCHack.app/Contents/MacOS/CCHack -appstore-compliant

cp -r share CCEdit.app/Contents/
cp -r share CC2Edit.app/Contents/
rm -r share
cp ../LICENSE* .

# Symlink the other packages to save space
#cd $DISTROOT/CCPlay.app
#rm -r Contents/Frameworks Contents/PlugIns
#ln -s ../CCEdit.app/Contents/Frameworks Contents/Frameworks
#ln -s ../CCEdit.app/Contents/PlugIns Contents/PlugIns
