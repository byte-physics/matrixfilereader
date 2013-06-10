#!/bin/sh

set -e

lastVersion=0.21
newVersion=0.22

filesToWatch="VC8 *.txt regression_tests *.pxp *.ihf"

if [ ! -z "$(git status -s --untracked-files=no $filesToWatch)" ]; then
	echo "Aborting, please commit the changes first"
	exit 0
fi

baseName=matrixFileReaderXOP-v$newVersion
folder=public-releases/$baseName
srcFolder=$folder/src
docFolder=$folder/doc
pxpFolder=$folder/pxp
regressFolder=$folder/tests

rm -rf $folder
rm -rf $zipfile

mkdir -p $folder
mkdir -p $srcFolder
mkdir -p $docFolder
mkdir -p $pxpFolder
mkdir -p $regressFolder

cp License.txt $folder
cp Readme-Compilation.txt $folder
cp INSTALL-public.txt $folder/INSTALL.txt
cp "matrixfilereader Help.ihf" $docFolder
cp matrixfilereader-basic-gui.pxp $pxpFolder

cp regression_tests/*.{bat,log,txt,ipf,pxp} $regressFolder

cp VC8/*.{bat,cpp,h,hpp,rc} VC8/MatrixFileReader.sln VC8/MatrixFileReader.vcproj $srcFolder

cp Operation-Template-Generator.pxp $srcFolder

cp VC8/Release/matrixfilereader.xop $folder
cp VC8/VC2005_Redist_package_x86/vcredist_x86.exe $folder


git rev-parse HEAD > $folder/internalVersionString.txt
rm -f changelog
git log --pretty="%B" $(cat public-releases/matrixFileReaderXOP-v$lastVersion/internalVersionString.txt)..HEAD $filesToWatch >> changelog
echo "################################################" >> changelog
echo "################################################" >> changelog
git log --stat $(cat public-releases/matrixFileReaderXOP-v$lastVersion/internalVersionString.txt)..HEAD $filesToWatch >> changelog

cd public-releases &&  zip -q -r $baseName.zip $baseName/* && cd ..

