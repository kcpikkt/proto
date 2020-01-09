#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/context.hh"
#include "proto/core/debug/common.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/graphics/gl.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/util/namespace-shorthands.hh"

namespace proto {


    #if 0
u64 Texture2D::serialized_data_size() {
    return size.x * size.y * channels * sizeof(u8);
}

    u64 Texture2D::serialized_size() {
    return
        next_multiple(16, sizeof(serialization::AssetHeader<Texture2D>)) +
        next_multiple(16, serialized_data_size());
}

serialization::AssetHeader<Texture2D> Texture2D::serialization_header_map() {
    serialization::AssetHeader<Texture2D> ret;
    ret.size = size;
    ret.channels = channels;
    ret.gpu_format = gpu_format;
    ret.format = format;
    ret.data_offset = sizeof(serialization::AssetHeader<Texture2D>);
    ret.data_size = serialized_data_size();
    return ret;
}

void Texture2D::_move(Texture2D&& other) {
    TextureInterface::_move(meta::move(other));
    _allocator = other._allocator;
    data       = other.data;
}

Texture2D::Texture2D(Texture2D&& other) {
    _move(meta::forward<Texture2D>(other));
}

Texture2D& Texture2D::operator=(Texture2D&& other) {
    _move(meta::forward<Texture2D>(other));
    return *this;
}

void Texture2D::init(void (*tex_param_config)()) {
    glGenTextures(1, &gl_id);
    assert(gl_id >= 0);

    // using binding function won't work on yet incomplete texture
    // so we are silently borrowing one slot just for a minute
    auto prev_bound_gl_id =
        context->texture_slots[0].bound_gl_id;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_id);

    // here goes custom config
    tex_param_config();

    glBindTexture(GL_TEXTURE_2D, prev_bound_gl_id);

    datatype = GL_UNSIGNED_BYTE;
}

void Texture2D::init(){
    glGenTextures(1, &gl_id);
    assert(gl_id >= 0);

    // using binding function won't work on yet incomplete texture
    // so we are silently borrowing one slot just for a minute
    auto prev_bound_gl_id =
        context->texture_slots[0].bound_gl_id;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_id);

    if(flags.check(TextureInterface::mipmap_bit))
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, prev_bound_gl_id);

    datatype = GL_UNSIGNED_BYTE;
    //gfx::stale_texture_slot(slot);
}

void Texture2D::init(ivec2 size, u32 gpu_format, u32 format, u32 datatype){
    init();
    this->gpu_format = gpu_format;
    this->format = format;
    this->size = size;
    this->datatype = datatype;
}

void Texture2D::init(void *data, ivec2 size, u32 gpu_format, u32 format, u32 datatype)
{
    init(size, gpu_format, format, datatype);
    assert(data);
    this->data = data;
}

void Texture2D::upload() {
    gfx::gpu_upload(this);
}
    #endif

} // namespace proto
