#!/bin/bash

# Environment mapping texture
# From Emil Persson, aka Humus (http://www.humus.name)

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TMP_DIR="$SCRIPT_DIR/.tmp"

fetch_resource() {
    local dst_dir="$1"
    local name="$2"
    local url="$3"

    if [ -d "$dst_dir/$name" ]; then
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

    mkdir -p "$dst_dir"
    mv "$extract_dir" "$dst_dir/$name"

    rm -f "$TMP_DIR/$name.zip"
    rm -rf "$extract_dir"

    echo "$name done."
}

echo "Fetching IBL texture..."
IBL_DIR="$SCRIPT_DIR/Textures/IBL"
fetch_resource "$IBL_DIR" "NissiBeach2" "https://www.humus.name/Textures/NissiBeach2.zip"

rmdir "$TMP_DIR" 2>/dev/null

echo "Done."
