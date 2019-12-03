#pragma once
#include "proto/core/debug.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/util/Bitfield.hh"

namespace proto {
    struct Texture;
namespace serialization {

    template<>
    struct AssetHeader<Texture> {
        ivec2 size;
        u8 channels;
        u64 data_offset;
        u64 data_size;
    };
} // namespace proto {

struct Texture : Asset{

    memory::Allocator * _allocator;
    u8 channels;
    u32 gl_id;
    s32 bound_unit = -1;
    void * data;
    ivec2 size;
    Bitfield<u8> flags;
    constexpr static u8 gpu_uploaded_bit = BIT(1);
    constexpr static u8 bound_bit        = BIT(2);

   
    u64 serialized_data_size() {
        return size.x * size.y * channels * sizeof(u8);
    }

    u64 serialized_size() {
        return
            next_multiple(16, sizeof(serialization::AssetHeader<Texture>)) +
            next_multiple(16, serialized_data_size());
    }

    serialization::AssetHeader<Texture> serialization_header_map() {
        serialization::AssetHeader<Texture> ret;
        ret.size = size;
        ret.channels = channels;
        ret.data_offset = sizeof(serialization::AssetHeader<Texture>);
        ret.data_size = serialized_data_size();
        return ret;
    }
     void init(){
        glGenTextures(1, &gl_id);
        assert(gl_id >= 0);
        glActiveTexture(GL_TEXTURE0);
        // FIXME(kacper):
        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
   
    void init(memory::Allocator * allocator){
        assert(allocator);
        _allocator = allocator;
        glGenTextures(1, &gl_id);
        assert(gl_id >= 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
};
} // namespace proto
