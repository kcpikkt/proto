#pragma once
#include "proto/core/StateCRTP.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/graphics/TextureInterface.hh"
#include "proto/core/asset-system/common.hh"

namespace proto {
    //    struct Texture2D;
    //namespace serialization {
    //
    //    template<>
    //    struct AssetHeader<Texture2D> {
    //        ivec2 size;
    //        u8 channels;
    //        u64 data_offset;
    //        u64 data_size;
    //        u32 gpu_format;
    //        u32 format;
    //    };
    //}

struct Texture2D : Asset{
    u8 channels; 

    u32 datatype = GL_UNSIGNED_BYTE;
    u32 format;
    u32 gpu_format;

    void * cached;

    u32 gl_id;
    s32 bound_unit = -1;
    ivec2 size;

    constexpr static u8 on_gpu_bit = BIT(0);
    constexpr static u8 cached_bit = BIT(1);
    constexpr static u8 bound_bit  = BIT(2);
    constexpr static u8 mipmap_bit  = BIT(3);
    Bitfield<u8> flags = 0;

    // batch id?
    // batch start index
    // batch index count
};




    #if 0

struct Texture2D :
        StateCRTP<Texture2D>,
        TextureInterface
{
    memory::Allocator * _allocator;
    void * data = nullptr;
   
    u64 serialized_data_size();
    u64 serialized_size();
    serialization::AssetHeader<Texture2D> serialization_header_map();

    void _move(Texture2D&& other);

    Texture2D() {}

    Texture2D(Texture2D&& other);

    Texture2D& operator=(Texture2D&& other);

    void init();
    void init(void (*tex_param_config)());
    void init(ivec2 size, u32 gpu_format, u32 format, u32 datatype = GL_UNSIGNED_BYTE);
    void init(void * data, ivec2 size, u32 gpu_format, u32 format, u32 datatype = GL_UNSIGNED_BYTE);
    void init(memory::Allocator * allocator);

    void upload();

    // chaining
    inline Texture2D& $_configure( void(*proc)(Texture2D&) ){
        proc(*this); return *this;
    }


    inline Texture2D& $_init(void (*tex_param_config)()) {
        init(tex_param_config); return *this;
    }

    inline Texture2D& $_init()
    {
        init(); return *this;
    }

    inline Texture2D& $_init(ivec2 size, u32 gpu_format, u32 format,
                             u32 datatype = GL_UNSIGNED_BYTE)
    {
        init(size, gpu_format, format, datatype); return *this;
    }

    inline Texture2D& $_init(void * data, ivec2 size, u32 gpu_format, u32 format,
                             u32 datatype = GL_UNSIGNED_BYTE)
    {
        init(data, size, gpu_format, format,datatype); return *this;
    }

    inline Texture2D& $_upload(){
        upload(); return *this;
    }

};
    #endif
} // namespace proto
