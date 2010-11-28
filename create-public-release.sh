#!/bin/sh

set -e

if [ ! -z "$(git status -s --untracked-files=no .)" ]; then
	echo "Aborting, please commit the changes first"
	exit 0
fi

folder=public-releases/new-release
srcFolder=$folder/src
docFolder=$folder/doc
pxpFolder=$folder/pxp

lastVersion=0.10

if [ -e "$folder" ]; then
	echo "Aborting, the folder $folder exists already"
	exit 0
fi

mkdir -p $folder
mkdir -p $srcFolder
mkdir -p $docFolder
mkdir -p $pxpFolder

cp License.txt $folder
cp INSTALL.txt Readme-Compilation.txt $folder
cp "matrixfilereader Help.ihf" $docFolder
cp matrixfilereader-basic-gui.pxp $pxpFolder

cp VC8/*.cpp VC8/*.h VC8/*.rc VC8/MatrixFileReader.sln VC8/MatrixFileReader.suo VC8/MatrixFileReader.vcproj $srcFolder

cp Operation-Template-Generator.pxp $srcFolder

cp VC8/Release/matrixfilereader.xop $folder
cp VC8/VC2005_Redist_package_x86/vcredist_x86.exe $folder

git rev-parse HEAD > $folder/versionString.txt
git log --pretty="%B" $(cat releases/$lastVersion/fullVersion.txt)..HEAD > changelog
