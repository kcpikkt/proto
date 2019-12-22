#include "proto/core/graphics/Cubemap.hh"
#include "proto/core/context.hh"


namespace proto {

serialization::AssetHeader<Cubemap> Cubemap::serialization_header_map() {
    serialization::AssetHeader<Cubemap> ret;
    ret.size = size;
    ret.channels = channels;
    ret.format = format;
    ret.gpu_format = gpu_format;
    ret.data_offset = sizeof(serialization::AssetHeader<Cubemap>);
    ret.data_size = serialized_data_size();
    return ret;
}

u64 Cubemap::serialized_data_size() {
    return size.x * size.y * channels * sizeof(u8) * 6;
}

u64 Cubemap::serialized_size() {
    return
        next_multiple(16, sizeof(serialization::AssetHeader<Cubemap>)) +
        next_multiple(16, serialized_data_size());
}


void Cubemap::_move(Cubemap&& other) {
    TextureInterface::_move(meta::move(other));
    for(u8 i=0; i<6; i++) {
        data[i] = other.data[i];
    }
}

Cubemap::Cubemap(Cubemap&& other) {
    _move(meta::forward<Cubemap>(other));
}

Cubemap& Cubemap::operator=(Cubemap&& other) {
    _move(meta::forward<Cubemap>(other));
    return *this;
}



void Cubemap::init() {
    glGenTextures(1, &gl_id);

    // using binding function won't work on yet incomplete texture
    // so we are silently borrowing one slot just for a minute
    auto prev_bound_gl_id =
        context->texture_slots[0].bound_gl_id;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gl_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, prev_bound_gl_id);

    datatype = GL_UNSIGNED_BYTE;
}

void Cubemap::init(ivec2 size, u32 format, u32 gpu_format, u32 datatype){
    init();
    this->format = format;
    this->gpu_format = gpu_format;
    this->size = size;
    this->datatype = datatype;
}

} // namespace proto
