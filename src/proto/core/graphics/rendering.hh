#pragma once
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/Material.hh"
#include "proto/core/graphics/gl.hh"

namespace proto {
namespace graphics {

void render_span(Mesh * mesh, u32 index, bool simple = false) {
    // NOTE(kacper): THIS FUNCTION DOES NOT BIND THE MESH

    assert(proto::context);
    assert(proto::context->current_shader);
    auto& ctx = *proto::context;

    if(!simple) {
        Material * material = &mesh->spans[index].material;
        ctx.current_shader->set_material(material);
    }

    ctx.current_shader->set_uniform<GL_UNSIGNED_INT> ("u_hash", mesh->handle.hash);
    ctx.current_shader->set_uniform<GL_UNSIGNED_INT> ("u_span_index", (u32)index);

    auto begin_index = mesh->spans[index].begin_index;
    auto index_count = mesh->spans[index].index_count;
    assert(begin_index < mesh->indices.size());
    assert(begin_index + index_count <= mesh->indices.size());

    //    gl::debug_print_texture_slots();

    glDrawElements (GL_TRIANGLES, index_count,
                    GL_UNSIGNED_INT, (void*)(sizeof(u32) * begin_index));

    //TODO(kacper): no need to stale all of them? perhaps stale only ones that
    //              you used?
    gl::stale_all_texture_slots();
}

void render_mesh(Mesh * mesh, bool simple = false) {
    assert(mesh);
    mesh->bind();
    for(u32 i=0; i<mesh->spans.size(); i++) render_span(mesh, i, simple);
}

} // namespace graphics
} // namespace proto
