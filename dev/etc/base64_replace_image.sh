#!/usr/bin/env sh

# Replace base64 encoded image data in Dern source file with new base64
# encoded data. Script finds a very long line and replaces that one with
# the new data (in one line too).

# $1 Path of the image to base64 encode.
# $2 Path of the Dern file to edit.

UNAME=$(uname -s)
SED=sed
GREP=grep
BASE64="base64 -w 0 $1"

if [[ "$UNAME" == 'OpenBSD' ]]; then
    SED=gsed
    GREP=ggrep
    BASE64="base64 $1"
fi

LINE_NUMBER=$("$GREP" -n  '^.\{500\}' $2 | cut -f1 -d:)
BASE64_DATA=$($BASE64)
BASE64_DATA=$(echo $BASE64_DATA | tr -d '\040\011\012\015')

echo "encoding:          $1\nreplacing line $LINE_NUMBER: $2"
"$SED" -i -f - $2 <<EOF  $2
${LINE_NUMBER}s_.*_        [${BASE64_DATA}]_
EOF
