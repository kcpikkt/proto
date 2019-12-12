#pragma once
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/Material.hh"
#include "proto/core/graphics/gl.hh"
#include "proto/core/math/geometry.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/entity-system/common.hh"

namespace proto {
namespace graphics {

// NOTE(kacper): this function does not bind the mesh
void render_span(Mesh * mesh, u32 index, bool simple = false) {

    if(index == 1 && index == 2) return;
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

    glDrawElements (GL_TRIANGLES, index_count,
                    GL_UNSIGNED_INT, (void*)(sizeof(u32) * begin_index));

    //TODO(kacper): no need to stale all of them? perhaps stale only ones that
    //              you used?
    if(!simple)
        stale_all_texture_slots();
}

void render_mesh(Mesh * mesh, bool simple = false) {
    assert(mesh);
    mesh->bind();
    for(u32 i=0; i< mesh->spans.size() ; i++) render_span(mesh, i, simple);
}

void render_quad(s32 texture_unit,
                 vec2 pos = vec2(0.0),
                 vec2 size = context->window_size) {
    auto& quad_shader = get_asset_ref<ShaderProgram>(context->quad_shader_h);
    quad_shader.use();

    size /= context->window_size;
    pos /= context->window_size; 
    pos *= 2.0f; // <- multiplying by 2 here instead of in matrix

    // premultiplied goto origin, scale goback, translate
    mat3 matrix = {{               size.x,                  0.0, 0.0},
                   {                  0.0,               size.y, 0.0},
                   { size.x + pos.x - 1.0, size.y + pos.y - 1.0, 1.0}};

    quad_shader.set_uniform<GL_FLOAT_MAT3>("u_matrix", &matrix);
    quad_shader.set_uniform<GL_SAMPLER_2D>("u_tex", texture_unit);
    get_asset_ref<Mesh>(proto::context->quad_h).bind();
   
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_shadowmaps() {}

void render_gbuffer() {
    auto& ctx = *context;

    if(glIsEnabled(GL_MULTISAMPLE))
        debug_warn(debug::category::graphics,
                   "Rendering to gbuffer with GL_MULTISAMPLE enabled.");

    auto gbuffer_shader = get_asset_ref<ShaderProgram>(ctx.gbuffer_shader_h);
    gbuffer_shader.use();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 model = mat4(1.0);
    model = glm::scale(model, vec3(0.1));
    model = rotate(model, (float)M_PI/2.0f, vec3(0.0, 1.0, 0.0));
    model = translate(model, vec3(0.0, -10.0, 0.0));

    mat4 projection = ctx.camera.projection_matrix();
    mat4 view = ctx.camera.view_matrix();
    mat4 mvp = projection * view * model;

    gbuffer_shader.set_uniform<GL_FLOAT>      ("u_time", ctx.clock.elapsed_time);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &mvp);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_model", &model);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_view", &view);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_projection", &projection);

    render_mesh(&ctx.meshes[1]);
}

void render_scene() {
    auto& ctx = *context;
    auto& time = ctx.clock.elapsed_time;
    assert(context->current_shader);

    assert(0 && "commented out");
    #if 0
    for(auto& comp : context->comp.render_mesh) {
        TransformComp * transform = get_component<TransformComp>(comp.entity);
        assert(transform);

        //mat4 model = translate(mat4(1.0), transform->position);
        Mesh * mesh = get_asset<Mesh>(comp.mesh);

        ctx.camera.position = vec3(0.0,0.0,00.0);
        mat4 model = mat4(1.0);
        model = scale(model, vec3(0.01));
        model = glm::rotate(model, (float)M_PI/2.0f, vec3(0.0,1.0,0.0));
        model = translate(model, vec3(0.0, -1.0, cos(time) * 1.0));
        mat4 mvp = ctx.camera.projection_matrix() * model;

        ctx.current_shader->set_uniform<GL_FLOAT>("u_time", time);
        ctx.current_shader->set_uniform<GL_FLOAT_MAT4>("u_mvp", &mvp);
        ctx.current_shader->set_uniform<GL_FLOAT_MAT4>("u_model", &model);

        render_mesh(mesh, true);
    }
    #endif
}

} // namespace graphics
} // namespace proto
