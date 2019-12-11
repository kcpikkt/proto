#pragma once
#include "proto/core/debug.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/graphics/TextureInterface.hh"

namespace proto {
    struct Texture2D;
namespace serialization {

    template<>
    struct AssetHeader<Texture2D> {
        ivec2 size;
        u8 channels;
        u64 data_offset;
        u64 data_size;
    };
} // namespace proto {

struct Texture2D :
        DataholderCRTP<Texture2D>,
        TextureInterface
{
    memory::Allocator * _allocator;
    void * data;
   
    u64 serialized_data_size() {
        return size.x * size.y * channels * sizeof(u8);
    }

    u64 serialized_size() {
        return
            next_multiple(16, sizeof(serialization::AssetHeader<Texture2D>)) +
            next_multiple(16, serialized_data_size());
    }

    serialization::AssetHeader<Texture2D> serialization_header_map() {
        serialization::AssetHeader<Texture2D> ret;
        ret.size = size;
        ret.channels = channels;
        ret.data_offset = sizeof(serialization::AssetHeader<Texture2D>);
        ret.data_size = serialized_data_size();
        return ret;
    }
    void _move(Texture2D&& other) {
        TextureInterface::_move(meta::move(other));
        _allocator = other._allocator;
        data       = other.data;
    }

    Texture2D() {}

    Texture2D(Texture2D&& other) {
        _move(meta::forward<Texture2D>(other));
    }

    Texture2D& operator=(Texture2D&& other) {
        _move(meta::forward<Texture2D>(other));
        return *this;
    }


    void init(){
        glGenTextures(1, &gl_id);
        assert(gl_id >= 0);
        glActiveTexture(GL_TEXTURE0);
        // FIXME(kacper):
        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void init(ivec2 size, u32 format, u32 gpu_format){
        init();
        this->format = format;
        this->gpu_format = gpu_format;
        this->size = size;
    }
   
    void init(memory::Allocator * allocator){
        assert(allocator);
        _allocator = allocator;
        glGenTextures(1, &gl_id);
        assert(gl_id >= 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
};
} // namespace proto
