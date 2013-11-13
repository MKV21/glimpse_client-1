#!/bin/sh

BASEDIR=$(dirname $0)
cd $BASEDIR

OUTFILE="qmlresources.qrc"

echo "<RCC>" > $OUTFILE
echo "	<qresource prefix=\"/\">" >> $OUTFILE

for file in $(find qml -name "*.*")
do
	echo "		<file>$file</file>" >> $OUTFILE
done

echo "	</qresource>" >> $OUTFILE
echo "</RCC>" >> $OUTFILE
