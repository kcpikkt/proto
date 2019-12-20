# proto [![Build Status](https://travis-ci.org/kcpikkt/proto.svg?branch=master)](https://travis-ci.org/kcpikkt/proto)
    proto is my bottom-up apprach to building game/demo framework.
    It is supposed to handle everything from interfacing with the platform, input, timekeeping, memory managment, 
    to sound, rendering, entity-component system, asset managment and more.
    In this early stage it doesn't do all of that well yet but I was capable to pull off a small demo: [link](https://youtu.be/WClkKQ8i9xY)

    It uses IdTech-like architecture with game/demo being dynamic library that can be hot-swapped at any time.
    It has its own asset file format - .past (.p(roto)a(s)sset?) which is basically just memory dump for faster loads
    and prouldy called cli under src/tools, that currently serves just as an parser from external formats to pasts.
 
# Dependencies
    proto itself depends only on native platform libraries, libc, gl3w and glm for maths
    (I want to implement my own maths with intrinsics but I am not there yet)
    but src/tools/proto-cli/ requires also assimp and std_image.
    It runs only on OpenGL 4 or newer drivers.

# Building
    proto is deep in development and it is developed on linux, therefore right now it only builds on linux,
    although is written with being multiplatform in mind.

    scarce dependencies let proto be built with just simple makefile. targets are:

    runtime - builds proto-runtime which takes path to client dynamic library it is supposed to run as its
    first command line argument.
    # make runtime
    # bin/proto-runtime path/to/my/game.so

    client - recursively builds .cc files under directory specified in client_src_dir parameter,
    then links them into dynamic library under bin/ with name specified in client_name parameter.
    Additionally creates script that invokes runtime with name of built library as first command line argument.
    # make client client_src_dir=src/demos/demo client_name=demo
    # bin/run-demo.sh

    proto tools are also meant to be built the same way.
    # make client client_src_dir=src/tools/proto-cli client_name=cli \
           client_includes=-I/usr/local/include client_libs="-rpath /usr/local/lib -lassimp" 
    # bin/proto-runtime bin/cli.so parse mesh /res-external/crytek-sponza/sponza.obj res/sponza/
   
    standalone - standalone build would be possible with full rebuild of all objects and would require just
    few preprocessor conditionals, though is still on my long todo list. 
