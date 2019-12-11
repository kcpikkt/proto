#!/bin/bash
make runtime
make client \
     client_src_dir=src/tools/proto-cli \
     client_name=cli \
     client_includes=-I/usr/local/include \
     client_libs="-rpath /usr/local/lib -lassimp" \
