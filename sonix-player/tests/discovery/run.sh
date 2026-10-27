#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
build_dir="$(mktemp -d /tmp/sonix-discovery-build-XXXXXX)"
flags=(-std=c11 -Wall -Wextra -Werror -g -O1 -D_FILE_OFFSET_BITS=64 -I. -pthread)
if [[ "${SANITIZE:-1}" == 1 ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
gcc "${flags[@]}" tests/discovery/test_discovery.c src/system/discovery/discovery.c src/system/discovery/download_queue.c src/system/core/json.c src/system/core/md5.c -Wl,--wrap=fstatvfs,--wrap=fstatvfs64 -o "$build_dir/test_discovery"
"$build_dir/test_discovery"
gcc "${flags[@]}" -DHOST_BUILD=1 tests/discovery/test_http.c src/system/discovery/discovery.c src/system/net/http.c src/system/net/tls.c src/system/core/json.c -ldl -o "$build_dir/test_http"
python3 tests/discovery/test_http_server.py "$build_dir/test_http"
if [[ "${LIVE:-0}" == 1 ]]; then "$build_dir/test_http" --live "${QUERY:-Bach}"; fi
