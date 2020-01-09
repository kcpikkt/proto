#pragma once
#include "proto/core/common.hh"
#include "proto/core/context.hh"
#include "proto/core/common/types.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/entity-system/components.hh"
#include "proto/core/entity-system/interface.hh"
#include "proto/core/asset-system/interface.hh"

namespace proto {

struct RenderBatch {
    struct Range {
        u64 begin;
        u64 size;
    };

    u32 vertex_buffer, vertex_attrib, index_pointer;
    u64 vertex_buffer_size;
    u64 index_buffer_size;

    //    u64 vertex_buffer_largest_range_idx;
    //    u64 index_buffer_largest_range_idx;

    Array<RenderMeshComp> render_mesh_comps;
    Array<Mesh> meshes;

    Array<Range> free_vertex_ranges;
    Array<Range> free_index_ranges;

    void init(u64 _vertex_buffer_size, u64 _index_buffer_size) {
        vertex_buffer_size = _vertex_buffer_size;
        index_buffer_size = _index_buffer_size;

        glGenVertexArrays(1, &vertex_attrib);
        glGenBuffers(1, &vertex_buffer);

        glBindVertexArray(vertex_attrib);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, NULL, GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                            (void*) (offsetof(struct Vertex, position)) );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                                (void*) (offsetof(struct Vertex, normal)) );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                                (void*) (offsetof(struct Vertex, uv)) );

        free_vertex_ranges.init(1, &context->memory);
        free_index_ranges.init(1, &context->memory);

        free_vertex_ranges.push_back( Range{0, vertex_buffer_size} );
        free_index_ranges.push_back( Range{0, index_buffer_size} );

        render_mesh_comps.init(&context->memory);
        meshes.init(&context->memory);

        //    vertex_buffer_largest_range_idx = index_buffer_largest_range_idx = 0;
    }

    //TODO(kcpikkt): check if we are bound
    void add(RenderMeshComp& render_mesh) {
        Mesh * mesh_ptr = get_asset<Mesh>(render_mesh.mesh_h);
        if(!mesh_ptr) {
            debug_warn(debug::category::data,
                       "Render Mesh Component passed to ", __PRETTY_FUNCTION__, " does not refer to any loaded mesh.");
            return;
        }

        if(auto mesh_idx_opt = meshes.contains(*mesh_ptr)) {
            // given mesh is already in our batch buffer, no need for data duplicate
            // handle stays the same except for index hint
            render_mesh_comps.push_back(render_mesh).mesh_h.idx_hint = meshes.size();
            render_mesh.flags.set(RenderMeshComp::batched_bit);
            return;
        }

        if(!mesh_ptr->flags.check(Mesh::cached_bit)) {
            // if is not cached, fetch data from disk
            debug_warn(debug::category::data, "Implement me");
            return;
        }

        proto_assert(mesh_ptr->cached);

        u64 vertex_mem = mesh_ptr->vertex_count * sizeof(Vertex);
        proto_assert(vertex_mem);
        //if(vertex_mem < free_vertex_ranges[vertex_buffer_largest_range_idx].size) {
        //    debug_warn(debug::category::data, "Not enough space in batch vertex mem");
        //    return;
        //}

        u64 index_mem = mesh_ptr->index_count * sizeof(u32);
        //if(index_mem < free_index_ranges[index_buffer_largest_range_idx].size) {
        //    debug_warn(debug::category::data, "Not enough space in batch index mem");
        //    return;
        //}

        // hinting our handle
        render_mesh_comps.push_back(render_mesh).mesh_h.idx_hint = meshes.size();
        render_mesh.flags.set(RenderMeshComp::batched_bit);

        auto& mesh = meshes.push_back(*mesh_ptr);
        mesh.batch_range_size = 0;

        for(u64 i=0; i<free_vertex_ranges.size(); ++i) {

            if(free_vertex_ranges[i].size >= vertex_mem) {
                mesh.batch_range_begin = free_vertex_ranges[i].begin;
                mesh.batch_range_size = vertex_mem;

                if(free_vertex_ranges[i].size == vertex_mem) {
                    free_vertex_ranges.erase(i);
                } else {
                    free_vertex_ranges[i].begin += vertex_mem;
                    free_vertex_ranges[i].size -= vertex_mem;
                }
            }
        }

        if(!mesh.batch_range_size){
            debug_warn(debug::category::data, "Could not fit mesh into RenderBatch buffer");
            return;
        }

        void * range_ptr =
            glMapBufferRange(GL_ARRAY_BUFFER, mesh.batch_range_begin, mesh.batch_range_size,
                             GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if(!range_ptr) {
            debug_warn(debug::category::data, "glMapBufferRange failed");
            return;
        }

        memcpy(range_ptr, mesh.cached, mesh.batch_range_size);

        // FIXME(kcpiktk): do that once after buffer is ready to render?
        if(glUnmapBuffer(GL_ARRAY_BUFFER) != GL_TRUE) {
            debug_warn(debug::category::data, "glUnmapBuffer failed");
            return;
        }
    }

    void render() {
        auto& ctx = *context;
        auto& time = ctx.clock.elapsed_time;

        mat4 mvp, model, view = ctx.camera.view(), projection = ctx.camera.projection();

        for(auto& render_mesh : render_mesh_comps) {
            u64 i=0;
            for(; i<meshes.size(); ++i) 
                if(meshes[i].handle == render_mesh.mesh_h) break;

            assert(i != meshes.size());
            auto& mesh = meshes[i];

            auto* transform = get_component<TransformComp>(render_mesh.entity);
            assert(transform);

            model = mat4(1.0);
            model = translate(model, transform->position) * glm::toMat4(transform->rotation);

            mvp = projection * view * model;

            assert(ctx.current_shader);

            (*ctx.current_shader)
                .$_set_mat4  ("u_mvp", &mvp)
                .$_set_vec3  ("u_color", &render_mesh.color);

            if(mesh.flags.check(Mesh::indexed_bit)){
                debug_warn(debug::category::data, "Implement me");
            } else {
                glDrawArrays (GL_TRIANGLES, mesh.batch_range_begin, mesh.batch_range_size / sizeof(Vertex));   
            }
        }
    }
};


} // namespace proto
