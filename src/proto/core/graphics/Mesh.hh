#pragma once
#include "proto/core/graphics/common.hh"
#include "proto/core/common/types.hh"
#include "proto/core/util/Range.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/graphics/Vertex.hh"
#include "proto/core/math/hash.hh"

namespace proto {

struct Mesh : Asset{
    MemBuffer cached;

    constexpr static u8 on_gpu_bit = BIT(0);
    constexpr static u8 cached_bit = BIT(1);
    constexpr static u8 archived_bit = BIT(2);
    constexpr static u8 indexed_bit = BIT(3);
    Bitfield<u8> flags = 0;

    u64 vertices_count;
    u64 indices_count;

    vec3 bounds;
};

template<>
struct serialization::AssetHeader<Mesh> {
    constexpr static u32 signature = AssetType<Mesh>::hash;
    u32 sig = signature;
    u64 datasize;
    vec3 bounds;

    u64 vertices_offset;
    u64 vertices_count;
    u64 vertices_size;

    u64 indices_offset;
    u64 indices_count;
    u64 indices_size;

    AssetHeader() {}

    inline AssetHeader(Mesh& mesh);
};

template<> inline u64 serialization::serialized_size(Mesh& mesh) {
    return
        sizeof(AssetHeader<Mesh>) +
        mesh.vertices_count * sizeof(Vertex) +
        mesh.indices_count * sizeof(u32);
}

inline serialization::AssetHeader<Mesh>::AssetHeader(Mesh& mesh) {
    // 16 alignment?
    bounds = mesh.bounds;
    vertices_offset = sizeof(AssetHeader<Mesh>);
    vertices_count = mesh.vertices_count;
    vertices_size = vertices_count * sizeof(Vertex);

    indices_offset = vertices_offset + vertices_size;
    indices_count = mesh.indices_count;
    indices_size = indices_count * sizeof(u32);

    datasize = indices_offset + indices_size;

    assert(datasize == serialized_size(mesh));
}




#if 0
struct Mesh : Asset, StateCRTP<Mesh>{
    struct Span {
        u32 begin_index;
        u32 index_count;
        Material material;
        Bitfield<u8> flags;
        constexpr static u8 flip_uv_bit = BIT(0);
    };

    using State = StateCRTP<Mesh>;
    u32 VAO, VBO, EBO;

    // TODO(kacper): is on gpu
    memory::Allocator * _allocator;
    proto::Array<struct Vertex> vertices;
    proto::Array<u32> indices;
    proto::Array<Span> spans;

    Bitfield<u8> flags;
    constexpr static u8 on_gpu_bit = BIT(0);


    u64 serialized_vertices_size() {
        return vertices.size() * sizeof(decltype(vertices)::DataType);
    }

    u64 serialized_indices_size() {
        return indices.size() * sizeof(decltype(indices)::DataType);
    }

    u64 serialized_spans_size() {
        return spans.size() * sizeof(decltype(spans)::DataType);
    }

    u64 serialized_size() {
        return (next_multiple(16, sizeof(serialization::AssetHeader<Mesh>)) +
                next_multiple(16, serialized_vertices_size()) +
                next_multiple(16, serialized_indices_size()) +
                next_multiple(16, serialized_spans_size()) );
    }

    serialization::AssetHeader<Mesh> serialization_header_map(){
        serialization::AssetHeader<Mesh> ret;
        ret.vertices_offset =
            next_multiple(16, sizeof(serialization::AssetHeader<Mesh>));
        ret.vertices_count = vertices.size();
        ret.vertices_size = serialized_vertices_size();

        ret.indices_offset = ret.vertices_offset + ret.vertices_size;
        ret.indices_count = indices.size();
        ret.indices_size = serialized_indices_size();

        ret.spans_offset = ret.indices_offset + ret.indices_size;
        ret.spans_count = spans.size();
        ret.spans_size = serialized_spans_size();
        
        return ret;
    }

    void _move(Mesh&& other) {
        vertices = meta::move(other.vertices);
        indices = meta::move(other.indices);
        spans = meta::move(other.spans);

        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        _allocator = other._allocator;
    }

    Mesh() {}

    Mesh(Mesh&& other) {
        _move(meta::forward<Mesh>(other));
    }

    Mesh& operator=(Mesh&& other) {
        _move(meta::forward<Mesh>(other));
        return *this;
    }

    //FIXME(kacper);
    void init(memory::Allocator * allocator){
        State::state_init();
        assert(allocator);
        _allocator = allocator;
        vertices.init(_allocator);
        indices.init(_allocator);
        spans.init(_allocator);

        glGenVertexArrays(1, &VAO);
        glGenBuffers     (1, &VBO);
        glGenBuffers     (1, &EBO);
    }

    //void destroy_shallow() {
    //    glDeleteVertexArrays(1, &VAO);
    //    glDeleteBuffers     (1, &VBO);
    //    glDeleteBuffers     (1, &EBO);
    //}

    //void destroy_deep() {

    //}

    void init(size_t vertices_cap,
              size_t indices_cap,
              size_t spans_cap,
              memory::Allocator * allocator)
    {
        assert(allocator);
        _allocator = allocator;
        vertices.init(vertices_cap, _allocator);
        indices.init(indices_cap, _allocator);
        spans.init(spans_cap, _allocator);

        glGenVertexArrays(1, &VAO);
        glGenBuffers     (1, &VBO);
        glGenBuffers     (1, &EBO);

        glBindVertexArray(VAO);
    }

    void gpu_upload() {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);


        glBufferData(GL_ARRAY_BUFFER,
                    sizeof(struct Vertex) * vertices.size(),
                    vertices.raw(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    sizeof(u32) * indices.size(),
                    indices.raw(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                            (void*) (offsetof(struct Vertex, position)) );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                                (void*) (offsetof(struct Vertex, normal)) );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                                (void*) (offsetof(struct Vertex, uv)) );

    }
    void bind() {
        glBindVertexArray(VAO);
    }

};   

#endif
} // namespace proto
