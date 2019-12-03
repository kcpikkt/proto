#pragma once
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/Material.hh"
#include "proto/core/graphics/gl.hh"

namespace proto {
namespace graphics {

void render_span(Mesh * mesh, u32 index, bool simple = false) {
    assert(proto::context);
    assert(proto::context->current_shader);
    auto& ctx = *proto::context;

    if(!simple) {
        Material * material = &mesh->spans[index].material;
        ctx.current_shader->set_material(material);
    }

    ctx.current_shader->set_uniform<GL_UNSIGNED_INT> ("u_hash", mesh->handle.hash);
    ctx.current_shader->set_uniform<GL_UNSIGNED_INT> ("u_span_index", (u32)index);

   glDrawElements
       (GL_TRIANGLES, mesh->spans[index].index_count,
        GL_UNSIGNED_INT, (void*)(sizeof(u32) * mesh->spans[index].begin_index));

   gl::free_texture_slots();
}

void render_mesh(Mesh * mesh, bool simple = false) {
    assert(mesh);
    mesh->bind();
    for(u32 i=0; i<mesh->spans.size(); i++) render_span(mesh, i, simple);
}

} // namespace graphics
} // namespace proto
