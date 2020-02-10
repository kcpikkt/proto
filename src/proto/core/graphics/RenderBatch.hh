#pragma once
#include "proto/core/common.hh"
#include "proto/core/debug.hh"
#include "proto/core/util/Range.hh"
#include "proto/core/context.hh"
#include "proto/core/common/types.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/entity-system/components.hh"
#include "proto/core/entity-system/interface.hh"
#include "proto/core/asset-system/interface.hh"

namespace proto {

struct RenderBatch {

    u32 vertex_buffer, index_buffer, vertex_attrib, index_pointer; // sry, what is index pointer?
    u64 vertex_buffer_size;
    u64 index_buffer_size;

    //    u64 vertex_buffer_largest_range_idx;
    //    u64 index_buffer_largest_range_idx;

    Array<RenderMeshComp> render_mesh_comps;

    struct RenderBatchMesh : Mesh {
        // batch identification
        Range batch_index_range;
        Range batch_vertex_range;

        u32 ref_count = 0;

        RenderBatchMesh(const Mesh& mesh) : Mesh::Mesh(mesh) {}
    };

    struct RenderBatchMaterial : Material {

        RenderBatchMaterial(const Material& material) : Material::Material(material) {}
    };


    Array<RenderBatchMesh> meshes;
    Array<RenderBatchMaterial> materials;

    Array<Range> free_vertex_ranges;
    Array<Range> free_index_ranges;


    void init(u64 _vertex_buffer_size, u64 _index_buffer_size) {
        vertex_buffer_size = _vertex_buffer_size;
        index_buffer_size = _index_buffer_size;

        glGenVertexArrays(1, &vertex_attrib);
        glGenBuffers(1, &vertex_buffer);
        glGenBuffers(1, &index_buffer);

        glBindVertexArray(vertex_attrib);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, NULL, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, NULL, GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                              (void*) (offsetof(struct Vertex, position)) );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                              (void*) (offsetof(struct Vertex, normal)) );

        //glEnableVertexAttribArray(2);
        //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
        //                      (void*) (offsetof(struct Vertex, tangent)) );

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
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
        auto& ctx = *context;
        Mesh * mesh_ptr = get_asset<Mesh>(render_mesh.mesh_h);

        if(!mesh_ptr) {
            debug_warn(debug::category::data,
                       "Render Mesh Component passed to ", __PRETTY_FUNCTION__, " does not refer to any loaded mesh.");
            return;
        }

        if(auto mesh_idx_opt = meshes.contains(*mesh_ptr)) {
            // given mesh is already in our batch buffer, no need for data duplicate
            // handle stays the same except for index hint
            meshes[mesh_idx_opt.value].ref_count++;

            render_mesh_comps.push_back(render_mesh).mesh_h.idx_hint = mesh_idx_opt.value;
            render_mesh.flags.set(RenderMeshComp::batched_bit);
            return;
        }

        static auto alloc_range =
            [](u64 size, Array<Range>& ranges) -> Optional<Range> {

                for(u64 i=0; i<ranges.size(); ++i) {
                    if(ranges[i].size >= size) {
                        defer {
                            if(ranges[i].size == size)
                                ranges.erase(i);
                            else
                                (ranges[i].begin += size, ranges[i].size -= size);
                        };

                        return Range{ ranges[i].begin, size };
                    }
                }       
                return {};
            };

        MemBuffer cached;
        if(mesh_ptr->flags.check(Mesh::cached_bit)) {
            cached = mesh_ptr->cached;
            proto_assert(cached);
            
        } else {
            //if(!mesh_ptr->flags.check(Mesh::archived_bit)) {
            //    log_error(debug::cateogory::data, "No way to obtain mesh data.");
            //    return;
            //}
            // if is not cached, fetch data from disk and proceed
            //debug_warn(debug::category::data, "Implement me");
            auto h = render_mesh.mesh_h;
            AssetMetadata* metadata = get_metadata(h);

            auto archive_idx =
                ctx.open_archives.keys.find_if( [&](u32 hash){ return hash == metadata->archive_hash; } );

            if(archive_idx == ctx.open_archives.size())
                { log_error(debug::category::data, "Mesh refers to an archive that is not open."); return;}

            auto& archive = ctx.open_archives.at_idx(archive_idx);

            auto idx = metadata->archive_node_idx_hint =
                archive.nodes.find_if( [&](ser::Archive::Node& node){ return node.handle.hash == h.hash; },
                                    metadata->archive_node_idx_hint );

            if(idx == archive.nodes.size())
                { log_error(debug::category::data, "Mesh is not preset in archive it refers to."); return;}

            cached = archive.get_node_memory(idx);
            // the great thing about it though is that we memcpy from mapped file right to mapped gpu memory!
        }

        auto& header = *((serialization::AssetHeader<Mesh>*)(cached.data));

        if(header.sig != serialization::AssetHeader<Mesh>::signature) {
            log_error(debug::category::data, "Cached mesh data header signature was corrupted, aborting."); return; }

        proto_assert(header.vertices_size);

        // hinting our handle
        render_mesh_comps.push_back(render_mesh).mesh_h.idx_hint = meshes.size();
        render_mesh.flags.set(RenderMeshComp::batched_bit);

        auto& mesh = meshes.push_back(*mesh_ptr);

        // WE ASSUME THAT OUR BUFFERS ARE ALREADY BOUND
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

        auto map_flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;

        if(header.vertices_size) {
            if( auto range_opt = alloc_range(header.vertices_size, free_vertex_ranges) )
                mesh.batch_vertex_range = range_opt.value;
            else
                return (void)debug_warn(debug::category::data, "Could not fit mesh into RenderBatch buffer");

            auto& [r_begin, r_size] = mesh.batch_vertex_range;

            if(void * mapped_range = glMapBufferRange(GL_ARRAY_BUFFER, r_begin, r_size, map_flags) )
                memcpy(mapped_range, (u8*)&header + header.vertices_offset, r_size);
            else
                return (void)debug_warn(debug::category::data, "glMapBufferRange failed");
        }

        if(header.indices_size) {
            mesh.flags.set(Mesh::indexed_bit);
            if( auto range_opt = alloc_range(header.indices_size, free_index_ranges) )
                mesh.batch_index_range = range_opt.value;
            else
                return (void)debug_warn(debug::category::data, "Could not fit mesh into RenderBatch buffer");

            auto& [r_begin, r_size] = mesh.batch_index_range;

            if(void * mapped_range = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, r_begin, r_size, map_flags) )
                memcpy(mapped_range, (u8*)&header + header.indices_offset, r_size);
            else
                return (void)debug_warn(debug::category::data, "glMapBufferRange failed");

        }

        // FIXME(kcpiktk): do that once after buffer is ready to render?
        if(glUnmapBuffer(GL_ARRAY_BUFFER) != GL_TRUE)
            return (void)debug_warn(debug::category::data, "glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER) failed");

        if(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER) != GL_TRUE)
            return (void)debug_warn(debug::category::data, "glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER) failed");

        // we have mesh in, now its time for our material

        Material * material = get_asset<Material>(render_mesh.material_h);
        if(material) {
        } else
            debug_warn(debug::category::data, "RenderMesh is not assigned any material.");
    }

    void render() {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        auto& ctx = *context;

        static float next_frame_far = 1000.0; // default init value, tweak to avoid black/slow frame
        ctx.camera.far = next_frame_far;
        printf("%f\r", next_frame_far); flush();
        next_frame_far = 0.0;

        mat4 mvp, model, view = ctx.camera.view(), projection = ctx.camera.projection();

        for(auto& render_mesh : render_mesh_comps) {
            auto i = meshes.find_if( [&](auto mesh){ return mesh.handle == render_mesh.mesh_h; } );

            assert(i != meshes.size());
            auto& mesh = meshes[i];

            auto* transform = get_comp<TransformComp>(render_mesh.entity);
            assert(transform);

            float cam2farvert_dist =
                glm::distance(transform->position, ctx.camera.position)+ glm::length(mesh.bounds);

            next_frame_far = max(next_frame_far, cam2farvert_dist);

            model = translate(scale(mat4(1.0), transform->scale), transform->position) * glm::toMat4(transform->rotation);

            mvp = projection * view * model;

            assert(ctx.current_shader);

            (*ctx.current_shader)
                .$_set_mat4  ("u_mvp", &mvp)
                .$_set_vec3  ("u_color", &render_mesh.color);

            if(mesh.flags.check(Mesh::indexed_bit)){
                auto& [r_begin, r_size] = mesh.batch_index_range;
                u32 offset = mesh.batch_vertex_range.begin / sizeof(Vertex);
                glDrawElementsBaseVertex (GL_TRIANGLES, r_size / sizeof(u32), GL_UNSIGNED_INT,
                                          (void*)(r_begin), offset);   
            } else {
                auto& [r_begin, r_size] = mesh.batch_vertex_range;
                glDrawArrays (GL_TRIANGLES, r_begin / sizeof(Vertex), r_size / sizeof(Vertex));   
            }
        }
    }
};


} // namespace proto
