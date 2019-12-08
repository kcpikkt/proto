#pragma once
#include "proto/core/asset-system/common.hh"
#include "proto/core/graphics/TextureInterface.hh"

namespace proto {

struct Cubemap : TextureInterface {
    void * data[6];

    void _move(Cubemap&& other) {
        TextureInterface::_move(meta::move(other));
        for(u8 i=0; i<6; i++) {
            data[i] = other.data[i];
        }
    }

    Cubemap() {}

    Cubemap(Cubemap&& other) {
        _move(meta::forward<Cubemap>(other));
    }

    Cubemap& operator=(Cubemap&& other) {
        _move(meta::forward<Cubemap>(other));
        return *this;
    }


    void init() {
        glGenTextures(1, &gl_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gl_id);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};

} // namespace proto
