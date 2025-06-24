#!/bin/bash

REPO_URL="https://github.com/KhronosGroup/glTF-Sample-Assets.git"
DST_DIR="glTFSampleAssets"

echo "Fetching/Updating $DST_DIR..."

if [ -d "$DST_DIR/.git" ]; then
    pushd "$DST_DIR" > /dev/null
    git pull
    popd > /dev/null
else
    echo "Cloning repository..."
    git clone "$REPO_URL" "$DST_DIR"
fi

echo "Done."

# Remove --depth=1 because it cannot update the repository
