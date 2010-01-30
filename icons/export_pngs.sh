#!/bin/bash

sizes="128 64 48 32 22 16"
# input: hi-<category>-<name>.svgz
# output: hi<size>-<category>-<name>.png

for icon in *.svgz; do
for size in $sizes; do
    png=ox$size-`basename $icon .svgz | sed -e 's/^oxsc-//'`.png
    inkscape --without-gui --export-png="$png" --export-dpi=72 --export-background-opacity=0 --export-width=$size --export-height=$size $icon > /dev/null
done
done

