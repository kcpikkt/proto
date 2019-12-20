#pragma once
#include "proto/core/asset-system/common.hh"
#include "proto/core/graphics/TextureInterface.hh"

namespace proto {
    struct Cubemap;
namespace serialization {

    template<>
    struct AssetHeader<Cubemap> {
        ivec2 size;
        u8 channels;
        u32 format;
        u32 gpu_format;
        u64 data_size;
        u64 data_offset;
    };
}

struct Cubemap : TextureInterface {
    void * data[6] = {};

    serialization::AssetHeader<Cubemap> serialization_header_map();

    u64 serialized_data_size();

    u64 serialized_size();

    void _move(Cubemap&& other);

    Cubemap() {}

    Cubemap(Cubemap&& other);

    Cubemap& operator=(Cubemap&& other);

    void init();

    void init(ivec2 size, u32 format, u32 gpu_format, u32 datatype);

    inline Cubemap& $_init(ivec2 size, u32 format, u32 gpu_format, u32 datatype){
        init( size, format, gpu_format, datatype); return *this;
    }

};

} // namespace proto
