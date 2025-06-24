#!/bin/bash

# Models downloaded from Morgan McGuire's Computer Graphics Archive https://casual-effects.com/data

# TODO: https://casual-effects.com/data/

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
