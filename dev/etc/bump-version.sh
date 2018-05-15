#!/usr/bin/env bash

CONFIG_FILE="dev/include/octaspire/dern/octaspire_dern_config.h"
STR_MAJOR="define OCTASPIRE_DERN_CONFIG_VERSION_MAJOR"
STR_MINOR="define OCTASPIRE_DERN_CONFIG_VERSION_MINOR"
STR_PATCH="define OCTASPIRE_DERN_CONFIG_VERSION_PATCH"

MAJOR=$(grep "$STR_MAJOR" "$CONFIG_FILE" | awk '{ print $3 }' | sed s/\"//g)
MINOR=$(grep "$STR_MINOR" "$CONFIG_FILE" | awk '{ print $3 }' | sed s/\"//g)
PATCH=$(grep "$STR_PATCH" "$CONFIG_FILE" | awk '{ print $3 }' | sed s/\"//g)

echo    "Current version is $MAJOR.$MINOR.$PATCH"

create_new_version() {
    MAJOR=$1
    MINOR=$2
    PATCH=$3
    NEW_MAJOR=$4
    NEW_MINOR=$5
    NEW_PATCH=$6

    echo "New version is     $NEW_MAJOR.$NEW_MINOR.$NEW_PATCH"

    echo "Updating $CONFIG_FILE"
    sed -i "s/$STR_MAJOR \"$MAJOR\"/$STR_MAJOR \"$NEW_MAJOR\"/" "$CONFIG_FILE"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    sed -i "s/$STR_MINOR \"$MINOR\"/$STR_MINOR \"$NEW_MINOR\"/" "$CONFIG_FILE"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    sed -i "s/$STR_PATCH \"$PATCH\"/$STR_PATCH \"$NEW_PATCH\"/" "$CONFIG_FILE"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
}

RELTYPE=$1

if [ $RELTYPE = major ]; then
    NEW_MAJOR=$((MAJOR + 1))
    NEW_MINOR=0
    NEW_PATCH=0

    create_new_version $MAJOR $MINOR $PATCH $NEW_MAJOR $NEW_MINOR $NEW_PATCH

elif [ $RELTYPE = minor ]; then
    NEW_MAJOR=$MAJOR
    NEW_MINOR=$((MINOR + 1))
    NEW_PATCH=0

    create_new_version $MAJOR $MINOR $PATCH $NEW_MAJOR $NEW_MINOR $NEW_PATCH

elif [ $RELTYPE = patch ]; then
    NEW_MAJOR=$MAJOR
    NEW_MINOR=$MINOR
    NEW_PATCH=$((PATCH + 1))

    create_new_version $MAJOR $MINOR $PATCH $NEW_MAJOR $NEW_MINOR $NEW_PATCH

else
    echo "========================================================="
    echo "= major, minor or patch was expected. Going to quit now ="
    echo "========================================================="
    exit 1
fi
