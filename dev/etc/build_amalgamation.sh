#!/usr/bin/env sh
UNAME="$(uname)"

if [ "$#" -gt 0 ]; then
    echo "Building with coverage ($1) for $UNAME..."
else
    echo "Building for $UNAME..."
fi

if [ "$UNAME" = "Linux" ]; then
    cd release && sh how-to-build/linux.sh            $1  > /dev/null && echo "Done.";
elif [ "$UNAME" = "Darwin" ]; then
    cd release && sh how-to-build/macOS.sh            $1  > /dev/null && echo "Done.";
elif [ "$UNAME" = "OpenBSD" ]; then
    cd release && sh how-to-build/OpenBSD.sh          $1  > /dev/null && echo "Done.";
elif [ "$UNAME" = "FreeBSD" ]; then
    cd release && sh how-to-build/FreeBSD.sh          $1  > /dev/null && echo "Done.";
elif [ "$UNAME" = "NetBSD" ]; then
    cd release && sh how-to-build/NetBSD.sh           $1  > /dev/null && echo "Done.";
elif [ "$UNAME" = "Minix" ]; then
    cd release && sh how-to-build/minix3.sh           $1  > /dev/null && echo "Done.";
elif [ "$UNAME" = "DragonFly" ]; then
    cd release && sh how-to-build/DragonFlyBSD.sh     $1  > /dev/null && echo "Done.";
elif [ "$UNAME" = "Haiku" ]; then
    if $(uname -a | grep -q x86_64); then
        cd release && sh how-to-build/haiku-x86_64.sh $1  > /dev/null && echo "Done.";
    else
        cd release && sh how-to-build/haiku.sh        $1  > /dev/null && echo "Done.";
    fi
elif [ "$UNAME" = "AROS" ]; then
    cd release && sh how-to-build/AROS.sh             $1  > /dev/null && echo "Done.";
else
    echo "This platform is not handled by Makefile at the moment.";
    echo "Please build using a script from 'release/how-to-build'.";
fi
