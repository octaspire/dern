#!/usr/bin/env sh

# Replace base64 encoded image data in Dern source file with new base64
# encoded data. Script finds a very long line and replaces that one with
# the new data (in one line too).

# $1 Path of the image to base64 encode.
# $2 Path of the Dern file to edit.

LINE_NUMBER=$(grep -n  '^.\{500\}' $2 | cut -f1 -d:)
BASE64_DATA=$(base64 -w 0 $1)

echo "encoding:          $1\nreplacing line $LINE_NUMBER: $2"
sed -i "${LINE_NUMBER}s_.*_        [${BASE64_DATA}]_" $2
