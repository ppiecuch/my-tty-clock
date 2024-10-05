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

LIBS="$(ncursesw6-config --libs) -lmad -lpulse -lpulse-simple"
OPTS="-std=c++14 -D_X_OPEN_SOURCE_EXTENDED -D__GNU_SOURCE -D_GNU_SOURCE -I modules -Wno-write-strings"
DBG="-g -DDEBUG -D_DEBUG"

echo "*==============="
echo "* OPTS = $OPTS"
echo "* LIBS = $LIBS"
echo "* DBG = $DBG"
echo "*==============="

g++ $DBG -o my-words-memo $OPTS main.cpp modules/simpleini/ConvertUTF.cpp $CURL $LIBS
g++ $DBG -o my-words-memo-cron $OPTS maincron.cpp modules/datetime/datetime.cpp $CURL
g++ $DBG -o my-words-memo-tts $OPTS maingtts.cpp
