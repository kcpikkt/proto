#pragma once
#include "proto/core/math/common.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/Material.hh"
#include "proto/core/graphics/gl.hh"
#include "proto/core/math/geometry.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/entity-system.hh"

namespace proto {
namespace graphics {

// NOTE(kacper): this function does not bind the mesh
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

    auto begin_index = mesh->spans[index].begin_index;
    auto index_count = mesh->spans[index].index_count;
    assert(begin_index < mesh->indices.size());
    assert(begin_index + index_count <= mesh->indices.size());

    glDrawElements (GL_TRIANGLES, index_count,
                    GL_UNSIGNED_INT, (void*)(sizeof(u32) * begin_index));

    //TODO(kacper): no need to stale all of them? perhaps stale only ones those
    //              you used?
    if(!simple)
        stale_all_texture_slots();
}

void render_mesh(Mesh * mesh, bool simple = false) {
    assert(mesh);
    mesh->bind();
    for(u32 i=0; i< mesh->spans.size() ; i++) {
        render_span(mesh, i, simple);
    }
}

void render_quad() {
    get_asset_ref<Mesh>(proto::context->quad_h).bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_cube() {
    get_asset_ref<Mesh>(proto::context->cube_h).bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void render_texture_quad(s32 texture_unit,
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

    render_quad();
}

void render_std_basis() {
    auto& ctx = *context;
    auto std_basis_shader = get_asset_ref<ShaderProgram>(ctx.std_basis_shader_h);

    mat4 model = mat4(1.0);
    model = glm::scale(model, vec3(1.0));
    mat4 projection = ctx.camera.projection();
    mat4 view = ctx.camera.view();
    mat4 mvp = projection * view * model;

    std_basis_shader
        .$_use()
        .$_set_mat4("u_mvp", &mvp);

    get_asset_ref<Mesh>(ctx.std_basis_h).bind();
    glDrawArrays(GL_LINES, 0, 6);
}

void render_skybox(s32 texture_unit) {
    auto& ctx = *context;
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    mat4 skybox_mvp =
        ctx.camera.projection() * mat4(mat3(ctx.camera.view()));

    get_asset_ref<ShaderProgram>(ctx.skybox_shader_h)
        .$_use()
        .$_set_mat4  ("u_mvp",    &skybox_mvp)
        .$_set_tex2D ("u_skybox", texture_unit);

    render_cube();

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void render_scene(bool simple = false) {
    auto& ctx = *context;
    auto& time = ctx.clock.elapsed_time;
    assert(context->current_shader);

    for(auto& comp : context->comp.render_mesh) {
        TransformComp * transform = get_component<TransformComp>(comp.entity);
        assert(transform);

        mat4 model = transform->model();

        Mesh * mesh = get_asset<Mesh>(comp.mesh);

        if(!mesh) {
            debug_error(1, "not good, fixme, too late for error message");
            continue;
        }

        mat4 projection = ctx.camera.projection();
        mat4 view = ctx.camera.view();
        mat4 mvp = projection * view * model;

        (*ctx.current_shader)
            //tmp
            .$_set_int   ("u_is_light",
                          (s32)(0 != get_component<PointlightComp>(comp.entity)))
            .$_set_float ("u_time",       time)
            .$_set_mat4  ("u_mvp",        &mvp)
            .$_set_mat4  ("u_model",      &model);

        render_mesh(mesh, simple);
    }
}

void render_gbuffer() {
    auto& ctx = *context;

    if(glIsEnabled(GL_MULTISAMPLE))
        debug_warn(debug::category::graphics,
                   "Rendering to gbuffer with GL_MULTISAMPLE enabled.");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 model = mat4(1.0);
    model = glm::scale(model, vec3(0.1));
    model = rotate(model, (float)M_PI/2.0f, vec3(0.0, 1.0, 0.0));
    model = translate(model, vec3(0.0, -10.0, 0.0));

    mat4 projection = ctx.camera.projection();
    mat4 view = ctx.camera.view();
    mat4 mvp = projection * view * model;

    get_asset_ref<ShaderProgram>(ctx.gbuffer_shader_h)
        .$_use()
        .$_set_float ("u_time",       ctx.clock.elapsed_time);

    render_scene();
}


} // namespace graphics
} // namespace proto
