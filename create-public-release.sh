#!/bin/sh

set -e

lastVersion=0.25.1
newVersion=0.26.0

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
cp Readme.md $folder
cp "matrixfilereader Help.ihf" $docFolder
cp matrixfilereader-basic-gui.pxp $pxpFolder

cp regression_tests/*.{bat,log,ipf,pxp} $regressFolder

cp VC8/*.{bat,cpp,h,hpp,rc} VC8/MatrixFileReader.sln VC8/MatrixFileReader.vcxproj $srcFolder
cp -r VC8/bstrlib $srcFolder

cp Operation-Template-Generator.pxp $srcFolder

cp VC8/Release/matrixfilereader.xop $folder
cp VC8/Release-x64/matrixfilereader-64.xop $folder
cp VC8/VC2013_Redist_package/*.exe $folder

cd public-releases &&  zip -q -r $baseName.zip $baseName/* && cd ..

git rev-parse HEAD > $folder/internalVersionString.txt
rm -f changelog
git log --pretty="%B" $(cat public-releases/matrixFileReaderXOP-v$lastVersion/internalVersionString.txt)..HEAD $filesToWatch >> changelog
echo "################################################" >> changelog
echo "################################################" >> changelog
git log --stat $(cat public-releases/matrixFileReaderXOP-v$lastVersion/internalVersionString.txt)..HEAD $filesToWatch >> changelog
