#!/bin/bash

APP_NAME="Orion Public Transit"
OUTPUT_DIR="build"
SRC_DIR="src"
RESOURCES_DIR="resources"
DAT_FILE="$OUTPUT_DIR/game.dat"
CC="gcc"
CFLAGS="-Wall -Wextra -std=c11 -I/usr/include/lua5.4"
LDFLAGS="-lm -lraylib -llua5.4"

mkdir -p "$OUTPUT_DIR"
rm -f "$DAT_FILE"

FILES=()
while IFS= read -r -d '' file; do
    FILES+=("$file")
done < <(find "$RESOURCES_DIR" -type f -print0)

NUM_FILES=${#FILES[@]}
printf "\\x$(printf %02x $((NUM_FILES & 0xFF)))\\x00\\x00\\x00" >> "$DAT_FILE"

OFFSET=0
for FILE in "${FILES[@]}"; do
    NAME="$FILE"
    NAME_LEN=${#NAME}
    SIZE=$(stat -c%s "$FILE")

    printf "\\x$(printf %02x $((NAME_LEN & 0xFF)))\\x00\\x00\\x00" >> "$DAT_FILE"
    echo -n "$NAME" >> "$DAT_FILE"

    for i in {0..7}; do
        BYTE=$(( (SIZE >> (8*i)) & 0xFF ))
        printf "\\x%02x" "$BYTE" >> "$DAT_FILE"
    done

    for i in {0..7}; do
        BYTE=$(( (OFFSET >> (8*i)) & 0xFF ))
        printf "\\x%02x" "$BYTE" >> "$DAT_FILE"
    done

    OFFSET=$((OFFSET + SIZE))
done

for FILE in "${FILES[@]}"; do
    cat "$FILE" >> "$DAT_FILE"
done

$CC $CFLAGS "$SRC_DIR/main.c" "$SRC_DIR/engine.c" -o "$OUTPUT_DIR/orion_public_transit" $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Compilation successful! Executable: $OUTPUT_DIR/orion_public_transit"
else
    echo "Compilation failed."
    exit 1
fi

echo "Build finished. Run './$OUTPUT_DIR/orion_public_transit' to play."
