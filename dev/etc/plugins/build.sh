#!/usr/bin/env bash
cat << EOF
This script is intended to be used while developing plugins. For normal use,
it is recommended to use the files inside release-directory. The file for
your platform in directory 'how-to-build' builds also all the binary plugins.
This script supports only couple of platforms. Use the release to get support
for many more.
EOF

case "$(uname -s)" in
    Linux*)  platform=Linux;;
    Darwin*) platform=MacOS;;
    *)       platform=Unknown;;
esac

echo "--- Building plugins in $platform ---"

if [ "$platform" == "Linux" ]; then
    echo "--- plugin openssl ---"
    gcc -g -Wall -Wextra -std=c99 -c -fPIC -I ../../release dern_openssl.c
    gcc -g -Wall -Wextra -std=c99 -shared -o libdern_openssl.so dern_openssl.o -lssl -lcrypto
fi


if [ "$platform" == "MacOS" ]; then
    echo "--- plugin openssl ---"
    clang -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release -I /usr/local/opt/openssl/include dern_openssl.c
    clang -g -Wall -Wextra -std=c99 -dynamiclib -o libdern_openssl.dylib dern_openssl.o -L /usr/local/opt/openssl/lib -lssl -lcrypto
fi
