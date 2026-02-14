#!/bin/bash

# Models downloaded from Morgan McGuire's Computer Graphics Archive https://casual-effects.com/data

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DST_DIR="$SCRIPT_DIR/DefaultModels"
TMP_DIR="$SCRIPT_DIR/.tmp"

fetch_model() {
    local name="$1"
    local url="$2"

    if [ -d "$DST_DIR/$name" ]; then
        echo "Skipping $name (already exists)"
        return
    fi

    echo "Downloading $name..."
    mkdir -p "$TMP_DIR"
    curl -L -o "$TMP_DIR/$name.zip" "$url"

    echo "Extracting $name..."
    local extract_dir="$TMP_DIR/${name}_extract"
    mkdir -p "$extract_dir"
    unzip -q "$TMP_DIR/$name.zip" -d "$extract_dir"

    mkdir -p "$DST_DIR"
    mv "$extract_dir" "$DST_DIR/$name"

    rm -f "$TMP_DIR/$name.zip"
    rm -rf "$extract_dir"

    echo "$name done."
}

fetch_model "CornellBox" "https://casual-effects.com/g3d/data10/common/model/CornellBox/CornellBox.zip"
fetch_model "FireplaceRoom" "https://casual-effects.com/g3d/data10/research/model/fireplace_room/fireplace_room.zip"
fetch_model "LivingRoom" "https://casual-effects.com/g3d/data10/research/model/living_room/living_room.zip"

rmdir "$TMP_DIR" 2>/dev/null

echo "Done."
