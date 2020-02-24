# proto [![Build Status](https://travis-ci.org/kcpikkt/proto.svg?branch=master)](https://travis-ci.org/kcpikkt/proto)
proto is my bottom-up apprach to building game/demo framework without dependencies.
a bit outdated demo: [link](https://youtu.be/WClkKQ8i9xY)

![70MB+ preview gif, may take a bit to load...](/prev0.gif)

Programs created with proto are build into dll/so and run using proto-rt, this allows for hot-swapping the library on recompile. Standalone builds are possible but not quite yet there.
 
# Dependencies
proto itself depends only on native platform libraries, some libc, gl3w (as it is just auto generated biolerplate) and glm for maths (I want to implement my own maths with intrinsics but I am not there yet). It requires OpenGL 4 Core or newer drivers.
proto-ar - archivizer for assets, in-game objects (clusters of Entities and Components) and such, additionally uses assimp and stb_image.

# Building
proto is deep in development and it is developed primarily on Linux and therefore right now it builds only on Linux,
though is written with being multiplatform in mind; has clearly separated platform layer.

Scarce dependencies allow proto to be built with just simple makefile.

make runtime - builds proto-runtime which takes path to client dynamic library, it is supposed to run, as its
first command line argument.
```sh
$ make runtime
$ bin/proto-rt path/to/my/game.so
```

make client - recursively builds .cc files under directory specified in client_src_dir parameter,
then links them into dynamic library under bin/ with name specified in client_name parameter.
Additionally creates script that invokes runtime with name of built library as first command line argument.
```sh
$ make client client_src_dir=src/demos/demo client_name=demo
$ bin/run-demo.sh
```

proto tools are also meant to be built the same way.
```sh
$ make client client_src_dir=src/tools/proto-ar client_name=ar \
       client_includes=-I/usr/local/include client_libs="-rpath /usr/local/lib -lassimp" 
$ bin/proto-rt bin/proto-rt.so -s /res-external/crytek-sponza/ -v -i sponza.obj -o res/sponza.pack
$ bin/proto-rt bin/proto-rt.so -ls res/sponza.pack
```
