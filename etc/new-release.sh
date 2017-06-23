#!/usr/bin/env sh

PROJECT_PATH=$1

echo "$PROJECT_PATH"

MAJOR=`grep "set(OCTASPIRE_DERN_CONFIG_VERSION_MAJOR" "$PROJECT_PATH/CMakeLists.txt" | awk '{ print $2 }' | sed s/\)//`
MINOR=`grep "set(OCTASPIRE_DERN_CONFIG_VERSION_MINOR" "$PROJECT_PATH/CMakeLists.txt" | awk '{ print $2 }' | sed s/\)//`
PATCH=`grep "set(OCTASPIRE_DERN_CONFIG_VERSION_PATCH" "$PROJECT_PATH/CMakeLists.txt" | awk '{ print $2 }' | sed s/\)//`

echo    ""
echo    "-------------- New release for Octaspire Dern --------------"
echo    ""
echo    "Current version is $MAJOR.$MINOR.$PATCH"
echo -n "Is this release major, minor or patch? [major/minor/patch]: "

create_new_version() {
    MAJOR=$1
    MINOR=$2
    PATCH=$3
    NEW_MAJOR=$4
    NEW_MINOR=$5
    NEW_PATCH=$6

    echo "New version is $NEW_MAJOR.$NEW_MINOR.$NEW_PATCH\n"

    echo "\nUpdating CMakeLists.txt...\n--------------------------\n"
    sed -i "s/set(OCTASPIRE_DERN_CONFIG_VERSION_MAJOR $MAJOR)/set(OCTASPIRE_DERN_CONFIG_VERSION_MAJOR $NEW_MAJOR)/" "$PROJECT_PATH/CMakeLists.txt"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    sed -i "s/set(OCTASPIRE_DERN_CONFIG_VERSION_MINOR $MINOR)/set(OCTASPIRE_DERN_CONFIG_VERSION_MINOR $NEW_MINOR)/" "$PROJECT_PATH/CMakeLists.txt"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    sed -i "s/set(OCTASPIRE_DERN_CONFIG_VERSION_PATCH $PATCH)/set(OCTASPIRE_DERN_CONFIG_VERSION_PATCH $NEW_PATCH)/" "$PROJECT_PATH/CMakeLists.txt"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    sed -i "s/Documentation for Octaspire Dern programming language version $MAJOR.$MINOR.$PATCH/Documentation for Octaspire Dern programming language version $NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/" "$PROJECT_PATH/doc/book/Programming_in_Octaspire_Dern.adoc"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    sed -i "s/Octaspire Dern version $MAJOR.$MINOR.$PATCH/Octaspire Dern version $NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/" "$PROJECT_PATH/test/REPL/octaspire-dern-repl.exp"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nRunning make...\n--------------------------\n"
    make
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nTesting...\n--------------------------\n"
    make test
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nBuilding book...\n--------------------------\n"
    make book-dern
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nGenerating amalgamation...\n--------------------------\n"
    "$PROJECT_PATH/etc/amalgamate.sh" "$PROJECT_PATH/etc"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCompiling amalgamation...\n--------------------------\n"
    gcc -std=c99 -Wall -Wextra -pedantic -Werror -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION -DGREATEST_ENABLE_ANSI_COLORS "$PROJECT_PATH/etc/octaspire_dern_amalgamated.c" -lm -o "$PROJECT_PATH/build/octaspire_dern_amalgamated_test_runner"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nTesting amalgamation...\n--------------------------\n"
    "$PROJECT_PATH/build/octaspire_dern_amalgamated_test_runner"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nRemoving old release directory and archive...\n--------------------------\n"
    rm -rf "$PROJECT_PATH/etc/release.tar.bz2"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    rm -rf "$PROJECT_PATH/etc/release"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCreating a directories for the source release...\n--------------------------\n"
    mkdir -p "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    mkdir -p "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/documentation"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    mkdir -p "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/examples"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    mkdir -p "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/tool-support"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    mkdir -p "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/plugins"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCreate a README file...\n--------------------------\n"
    echo \
"This is amalgamated single file source release for Octaspire Dern programming\n\
language version $NEW_MAJOR.$NEW_MINOR.$NEW_PATCH. File 'octaspire-dern-amalgamated.c'\n\
is all that is needed; it has no other dependencies than a C compiler and\n\
standard library supporting C99.\n\
\n\
SHA-512 checksums for this and older releases can be found from:\n\
https://octaspire.github.io/dern/\n\
If you want to check this release, download checksums for version $NEW_MAJOR.$NEW_MINOR.$NEW_PATCH from:\n\
https://octaspire.github.io/dern/checksums-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH\n\
\n\
Building instructions for all supported platforms (and scripts for building\n\
automatically) can be found in directory 'how-to-build'. Look for a file that\n\
has your platform's name in the file's name. If instructions for your\n\
platform are not yet added, looking instructions for a similar system will\n\
probably help. The amalgamation contains only one source file and should be\n\
straightforward to use. By using few compiler defines, the single file can\n\
be used for different purposes:\n\
\n\
\t(1) to build stand-alone unit test runner for the file.\n\
\t(2) to build stand-alone interactive Dern REPL.\n\
\t(3) to use the file as a single file header+library in C/C++ programs\n\
\t    wanting to embed the Dern language.\n\
\n\
Octaspire Dern is work in progress. The most recent version\n\
of this amalgamated source release can be downloaded from:\n\
\n\
\t* octaspire.com/dern/release.tar.bz2\n\
\t* https://octaspire.github.io/dern/release.tar.bz2\n\
\n\
Directory 'tool-support' contains files that help working with the Dern\n\
language using different tools; there are, for example, syntax files\n\
to allow Dern code to be syntax highlighted in Vim, Emacs and Pygments.\n\
\n\
Directory 'documentation' contains the book 'Programming in Octaspire Dern'\n\
and directory 'examples' has some short examples.\n\
\n\
More information about Dern can be found from the homepage:\n\
octaspire.com/dern\n" > "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/README"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCopying amalgamation...\n--------------------------\n"
    cp "$PROJECT_PATH/etc/octaspire_dern_amalgamated.c" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/octaspire-dern-amalgamated.c"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCopying LICENSE file...\n--------------------------\n"
    cp "$PROJECT_PATH/LICENSE" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/LICENSE"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCopying book to the release directory...\n--------------------------\n"
    cp "$PROJECT_PATH/doc/book/Programming_in_Octaspire_Dern.html" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/documentation/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    cp "$PROJECT_PATH/doc/book/Programming_in_Octaspire_Dern.pdf" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/documentation/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCopying plugins...\n--------------------------\n"
    cp "$PROJECT_PATH/etc/plugins/dern_ncurses.c" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/plugins/"

    echo "\nCopying files for binary library example...\n--------------------------\n"
    cp "$PROJECT_PATH/doc/examples/plugins/hello/amalgamated/mylib.c" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/examples/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    cp "$PROJECT_PATH/doc/examples/plugins/hello/use-mylib.dern" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/examples/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    echo "\nCopying file for binary 'dern_ncurses' plugin example...\n--------------------------\n"
    cp "$PROJECT_PATH/doc/examples/plugins/dern_ncurses/dern-ncurses-example.dern" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/examples/"

    echo "\nCopying embedding example...\n--------------------------\n"
    cp "$PROJECT_PATH/doc/examples/embedding-example.c" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/examples/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCopying build scripts to the release directory...\n--------------------------\n"
    cp -r "$PROJECT_PATH/etc/how-to-build/" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCopying tool-support directories...\n--------------------------\n"
    cp -r "$PROJECT_PATH/etc/emacs" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/tool-support/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    cp -r "$PROJECT_PATH/etc/vim" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/tool-support/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    cp -r "$PROJECT_PATH/etc/pygments" "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH/tool-support/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nCompressing release directory into tar.bz2...\n--------------------------\n"
    cd "$PROJECT_PATH/etc/"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    tar --bzip2 -cf "release.tar.bz2" release
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nRemoving $PROJECT_PATH/release/ and creating it again with updates\n--------------------------\n"
    rm -rf "$PROJECT_PATH/release"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    cp -r "$PROJECT_PATH/etc/release/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH" "$PROJECT_PATH"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi
    mv "$PROJECT_PATH/version-$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH" "$PROJECT_PATH/release"
    RETVAL=$?; if [ $RETVAL != 0 ]; then exit $RETVAL; fi

    echo "\nRelease $NEW_MAJOR.$NEW_MINOR.$NEW_PATCH created."
}

read RELTYPE

if [ $RELTYPE = major ]; then
    echo "\nMAJOR RELEASE\n-------------"
    NEW_MAJOR=$((MAJOR + 1))
    NEW_MINOR=0
    NEW_PATCH=0

    create_new_version $MAJOR $MINOR $PATCH $NEW_MAJOR $NEW_MINOR $NEW_PATCH

elif [ $RELTYPE = minor ]; then
    echo "\nMINOR RELEASE\n-----------"
    NEW_MAJOR=$MAJOR
    NEW_MINOR=$((MINOR + 1))
    NEW_PATCH=0

    create_new_version $MAJOR $MINOR $PATCH $NEW_MAJOR $NEW_MINOR $NEW_PATCH

elif [ $RELTYPE = patch ]; then
    echo "\nPATCH RELEASE"
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
