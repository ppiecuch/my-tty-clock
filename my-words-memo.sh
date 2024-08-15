#!/bin/bash

set -e

trap cleanup SIGINT

cleanup() {
    clear
    [[ -f previous_font ]] && setfont previous_font
   	setterm --reset
}

setfont -O previous_font # Store current font
setfont Uni3-TerminusBold32x16 # Set large font 1st (480x320+)

./my-words-memo
