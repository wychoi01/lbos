#!/bin/bash

mkdir -p temp_initrd

echo "Hello, world!" > temp_initrd/hello.txt
echo "This is a test file for the initrd filesystem." > temp_initrd/test.txt
echo "LBOS README" > temp_initrd/README.txt

cat > temp_initrd/test.c << 'EOL'
#include <stdio.h>

int main() {
    printf("Hello from initrd!\n");
    return 0;
}
EOL

if [ ! -f "tools/create_initrd" ]; then
    echo "Building initrd tool..."
    mkdir -p tools
    gcc -o tools/create_initrd tools/create_initrd.c
fi

./tools/create_initrd initrd.img temp_initrd/hello.txt temp_initrd/test.txt temp_initrd/README.txt temp_initrd/test.c

mkdir -p iso/boot/
cp initrd.img iso/boot/

echo "Inspecting initrd.img:"
hexdump -C initrd.img | head -20

echo "Test initrd image created at initrd.img and copied to iso/boot/"

# Cleanup
echo "Cleaning up temporary files..."
rm -rf temp_initrd

echo "Done!"
