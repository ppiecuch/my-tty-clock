#!/bin/bash

set -e

trap cleanup SIGINT

cleanup() {
    clear
    [[ -f previous_font ]] && setfont previous_font
    rm -f previous_font
   	setterm --reset
}

if [[ $OSTYPE == "linux-gnu"* ]]; then
  if [[ $EUID != 0 ]]; then 
    echo "Please run $(basename $0) as root"
    exit 0
  fi
fi

setfont -O previous_font # Store current font
setfont Uni3-TerminusBold32x16 # Set large font 1st (480x320+)

./my-words-memo

cleanup
