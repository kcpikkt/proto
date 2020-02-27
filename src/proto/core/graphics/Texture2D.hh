#pragma once
#include "proto/core/StateCRTP.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/graphics/TextureInterface.hh"
#include "proto/core/asset-system/common.hh"

namespace proto {

struct Texture2D : Asset{
    u8 channels; 

    u32 gl_datatype = GL_UNSIGNED_BYTE;
    u32 format;
    u32 gpu_format;

    u32 gl_id;
    s32 bound_unit = -1;
    ivec2 size;

    enum {
        bound_bit = asset_free_flags_bit,
        mipmap_bit = asset_free_flags_bit,
    };

    // batch id?
    // batch start index
    // batch index count
};

template<>
struct serialization::AssetHeader<Texture2D> {
    constexpr static u32 signature = AssetType<Texture2D>::hash;
    u32 sig = signature;
    u64 datasize;

    u8 channels; 
    u32 gl_datatype = GL_UNSIGNED_BYTE;
    u32 format;
    u32 gpu_format;
    ivec2 size;

    u64 pixeldata_offset;
    u64 pixeldata_size;

    AssetHeader() {}
    inline AssetHeader(Texture2D& texture);
};

template<> inline u64 serialization::serialized_size(Texture2D& texture) {
    u32 pix_sz;
    switch(texture.gl_datatype) {
    case(GL_UNSIGNED_BYTE):
        pix_sz = sizeof(u8); break;
    case(GL_UNSIGNED_INT):
        pix_sz = sizeof(u32); break;
    case(GL_FLOAT):
        pix_sz = sizeof(float); break;
    default:
        assert(0);
    }
    return sizeof(AssetHeader<Texture2D>) +
           texture.size.x * texture.size.y * pix_sz;
}

inline serialization::AssetHeader<Texture2D>::AssetHeader(Texture2D& texture)  :
    channels(texture.channels),
    gl_datatype(texture.gl_datatype),
    format(texture.format),
    gpu_format(texture.gpu_format),
    size(texture.size)
{
    pixeldata_offset = sizeof(AssetHeader<Texture2D>);
    u32 pix_sz;
    switch(gl_datatype) {
    case(GL_UNSIGNED_BYTE):
        pix_sz = sizeof(u8); break;
    case(GL_UNSIGNED_INT):
        pix_sz = sizeof(u32); break;
    case(GL_FLOAT):
        pix_sz = sizeof(float); break;
    default:
        assert(0);
    }
    pixeldata_size = size.x * size.y * pix_sz;
    datasize = pixeldata_offset + pixeldata_size;

    assert(datasize == serialized_size(texture));
}
 

} // namespace proto



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
