#!/bin/bash

set -e

if [[ $(uname -o) == "Darwin" ]]; then
  CURL="-framework Foundation -framework SystemConfiguration"
  for lib in curl psl ssl crypto nghttp2 idn2 unistring brotlidec-static brotlicommon-static iconv zstd z; do
    CURL="$CURL /opt/local/lib/lib$lib.a"
  done
else
  CURL="-lcurl"
fi

LIBS="$(ncurses6-config --libs) $(caca-config --libs)"
OPTS="-I modules"

g++ -g -o my-words-memo $OPTS *.cpp modules/simpleini/ConvertUTF.c $CURL $LIBS
