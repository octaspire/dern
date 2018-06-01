#!/usr/bin/env bash

CONFIG_FILE="dev/include/octaspire/dern/octaspire_dern_config.h"
STR_MAJOR="define OCTASPIRE_DERN_CONFIG_VERSION_MAJOR"
STR_MINOR="define OCTASPIRE_DERN_CONFIG_VERSION_MINOR"
STR_PATCH="define OCTASPIRE_DERN_CONFIG_VERSION_PATCH"

MAJOR=$(grep "$STR_MAJOR" "$CONFIG_FILE" | awk '{ print $3 }' | sed s/\"//g)
MINOR=$(grep "$STR_MINOR" "$CONFIG_FILE" | awk '{ print $3 }' | sed s/\"//g)
PATCH=$(grep "$STR_PATCH" "$CONFIG_FILE" | awk '{ print $3 }' | sed s/\"//g)

echo "Tag version $MAJOR.$MINOR.$PATCH"
git tag -a "v$MAJOR.$MINOR.$PATCH" -m "Dern version $MAJOR.$MINOR.$PATCH"
