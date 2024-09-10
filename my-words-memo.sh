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


font="28x14"

while :
do
  case "$1" in
    -f | --font)
        case "$2" in
          small) font="24x12" ;;
          medium) font="28x14" ;;
          big) font="32x16" ;;
          *) echo "Ignore parameter: $2"
        esac
        shift 2
        ;;
    --) # End of all options
        shift
        break
        ;;
    -*)
        echo "Error: Unknown option: $1" >&2
        exit 1
        ;;
    *)  # No more options
        break
        ;;
    esac
done


setfont Uni3-TerminusBold$font

if [ -f /tmp/words-memo.txt ]; then
  mv /tmp/words-memo.txt /tmp/words-memo.0.txt
fi

./my-words-memo -c

cleanup
