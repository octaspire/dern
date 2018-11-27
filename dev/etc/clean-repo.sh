echo "Write 'yes' to really clean the repo: "
read REPLY
echo ""
if [ "$REPLY" = "yes" ]
then
    echo "-- Cleaning git repository and submodules..."
    git clean -xfd
    git submodule foreach --recursive git clean -xfd
    git submodule update --init --recursive
    echo "OK  Done."
fi
