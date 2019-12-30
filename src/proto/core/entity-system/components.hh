#pragma once

namespace proto {

struct Component {
    Entity entity;
};

struct PointlightComp : Component {
    vec3 color = vec3(1.0);
    AssetHandle shadow_map;
};

struct TransformComp : Component {
    vec3 position = vec3(0.0f);
    quat rotation;
    // do not use nonuniform scale, not handled yet (why whould you even)
    vec3 scale = vec3(1.0f);

    inline mat4 model() {
        mat4 model = mat4(1.0);
        model = glm::scale(model, scale);
        model = glm::toMat4(rotation) * model;
        model = translate(model, position);
        return model;
    }
};

struct RenderMeshComp : Component {
    AssetHandle mesh;
    AssetHandle material;
};

} // namespace proto
