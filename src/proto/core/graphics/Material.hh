#pragma once
#include "proto/core/common.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/reflection.hh"

// 0		Color on and Ambient off
// 1		Color on and Ambient on
// 2		Highlight on
// 3		Reflection on and Ray trace on
// 4		Transparency: Glass on
// 		    Reflection: Ray trace on
// 5		Reflection: Fresnel on and Ray trace on
// 6		Transparency: Refraction on
// 		    Reflection: Fresnel off and Ray trace on
// 7		Transparency: Refraction on
// 		    Reflection: Fresnel on and Ray trace on
// 8		Reflection on and Ray trace off
// 9		Transparency: Glass on
// 		    Reflection: Ray trace off
// 10		Casts shadows onto invisible surfaces

namespace proto{

struct GLSLMaterialFieldRefl {
    const char * glsl_type;
    const char * glsl_name;
};

struct Material : Asset {;
    Material() {}

    constexpr static u8 pbr_material = 0;
    constexpr static u8 trad_material = 1;

    u8 type = pbr_material;

    struct PBR {
        REFL_FIELDS(Material, GLSLMaterialFieldRefl)
        (
        (AssetHandle) (albedo_map)    ({"sampler2D", "albedo_map"}),
        (AssetHandle) (metallic_map)  ({"sampler2D", "metallic_map"}),
        (AssetHandle) (roughness_map) ({"sampler2D", "roughness_map"}),
        (AssetHandle) (normal_map)    ({"sampler2D", "normal_map"}),
        (AssetHandle) (height_map)    ({"sampler2D", "height_map"}),
        (AssetHandle) (ao_map)        ({"sampler2D", "ao_map"})
        );
    };

    struct Trad {
        REFL_FIELDS(Material, GLSLMaterialFieldRefl)
        (
        (float) (shininess)  ({"float", "shininess"}),
        (vec3) (diffuse)  ({"vec3", "diffuse"}),
        (vec3) (specular) ({"vec3", "specular"}),
        (vec3) (ambient)  ({"vec3", "ambient"}),
        (AssetHandle) (diffuse_map)  ({"sampler2D", "diffuse_map"}),
        (AssetHandle) (specular_map) ({"sampler2D", "specular_map"}),
        (AssetHandle) (normal_map)   ({"sampler2D", "normal_map"}),
        (AssetHandle) (height_map)   ({"sampler2D", "height_map"})
        );
    };

    union {
        PBR pbr;
        Trad trad;
    };
};

template<>
struct serialization::AssetHeader<Material> : Material{
    constexpr static u32 signature = AssetType<Material>::hash;
    u32 sig = signature;
    u64 datasize;

    AssetHeader() {}

    inline AssetHeader(const Material& material) : Material::Material(material) {}
};

template<> inline u64 serialization::serialized_size(Material&) {
    return sizeof(AssetHeader<Material>);
}

    //inline serialization::AssetHeader<Material>::AssetHeader(Material& material){
    //    datasize = serialized_size(material);
    //    /**/ if(material.type == Material::pbr_material) 
    //        pbr = material.pbr;
    //    else if(material.type == Material::trad_material) 
    //        trad = material.trad;
    //    else
    //        assert(0);
    //}



} //namespace proto 
