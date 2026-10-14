#!/bin/bash
set -e

rm -f app

cc -I. $(find . -name '*.c' -not -path './tests/*') $(pkg-config --cflags --libs sdl3) -o app

./app