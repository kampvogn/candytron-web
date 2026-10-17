#!/bin/sh
# Build Candytron (fr-030, Farbrausch's GenThree player) to WebAssembly:
# web/build/candytron.js and candytron.wasm, next to index.html for serving.
#
#   git submodule update --init      # web/d3d9-webgl
#   web/build.sh
#
# Runs Emscripten in Docker. Env: IMAGE (emscripten/emsdk:4.0.10), OUT (build,
# relative to web/), EXTRA_LDFLAGS (e.g. --profiling-funcs), CFLAGS_EXTRA.
set -e
WEB=$(cd "$(dirname "$0")" && pwd)
ROOT=$(dirname "$WEB")
OUT=${OUT:-build}
mkdir -p "$WEB/$OUT"
python3 -I "$WEB/mkdata.py" "$ROOT/genthree/data" "$WEB/$OUT/data.cpp"
docker run --rm -u "$(id -u):$(id -g)" -e HOME=/tmp -e EM_CACHE=/tmp/emcache -e OUT="web/$OUT" \
  -e EXTRA_LDFLAGS="${EXTRA_LDFLAGS:-}" -e CFLAGS_EXTRA="${CFLAGS_EXTRA:-}" \
  -v "$ROOT":/w -w /w/genthree ${IMAGE:-emscripten/emsdk:4.0.10} sh -c '
COMMON="-sUSE_SDL=2 ${CFLAGS_EXTRA:-} -fpermissive -w -D_PLAYER -fms-extensions -fdeclspec -include /w/web/compat.h -I/w/web/shim -I/w/web/d3d9-webgl -O2"
fail=0
for f in v2synth v2web viruz2web _types genbitmap genmesh genmaterial genfx genmisc genplayer cslrt ronan mainplayer _startintro _start; do
  case $f in
    genplayer|cslrt) STD="-std=gnu++17" ;;          # scriptthunk.hpp
    v2synth|v2web) STD="-std=gnu++17 -I/w/v2" ;;    # V2: synth_core.cpp, v2mplayer.cpp
    *) STD="-std=c++98" ;;
  esac
  if em++ $STD $COMMON -c $f.cpp -o /w/$OUT/$f.o 2> /w/$OUT/$f.err; then echo "ok   $f"
  else echo "FAIL $f (see $OUT/$f.err)"; fail=1; fi
done
em++ -std=gnu++17 -O2 -w -I/w/web/d3d9-webgl -c /w/web/d3d9-webgl/d3d9.cpp -o /w/$OUT/d3d9.o && echo "ok   d3d9-webgl" || fail=1
em++ -std=gnu++17 -O1 -c /w/$OUT/data.cpp -o /w/$OUT/data.o && echo "ok   data" || fail=1
[ $fail = 0 ] || exit 1
cd /w/$OUT
em++ -O2 *.o -o candytron.js -sUSE_SDL=2 -sUSE_WEBGL2=1 -sFULL_ES3=1 -sMAX_WEBGL_VERSION=2 \
  -sASYNCIFY -sASYNCIFY_STACK_SIZE=1048576 -sSTACK_SIZE=8MB -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=256MB \
  -sENVIRONMENT=web -sINVOKE_RUN=0 -sEXIT_RUNTIME=0 -sEXPORTED_RUNTIME_METHODS=callMain $EXTRA_LDFLAGS
echo "built $OUT/candytron.wasm ($(wc -c < candytron.wasm) bytes)"'
cp "$WEB/index.html" "$WEB/$OUT/"
