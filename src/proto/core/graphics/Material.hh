#pragma once
#include "proto/core/common.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/asset-system/common.hh"

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

struct Material;

namespace serialization{
    //template<>
    //struct AssetHeader<Material> {
    //    proto::vec3 ambient_color  = proto::vec3(0.2);
    //    proto::vec3 diffuse_color  = proto::vec3(0.8);
    //    proto::vec3 specular_color = proto::vec3(1.0);

    //    AssetHandle specular_map;
    //    AssetHandle diffuse_map;
    //    AssetHandle ambient_map;
    //    AssetHandle bump_map;
    //};
} // namespace serialization
struct Material : Asset {

    //serialization::AssetHeader<Material> serialization_header_map() {
    //    serialization::AssetHeader<Material> ret;
    //    ret.ambient_color = ambient_color;
    //    ret.diffuse_color = diffuse_color;
    //    ret.specular_color = specular_color;

    //    ret.ambient_map = ambient_map;
    //    ret.diffuse_map = diffuse_map;
    //    ret.specular_map = specular_map;
    //    ret.bump_map = bump_map;
    //    return ret;
    //}
    //u64 serialized_size() {
    //    return sizeof();
    //}

    //tmp, use bitfield
    bool transparency = false;

    proto::vec3 emission_color  = proto::vec3(0.0);
    proto::vec3 ambient_color  = proto::vec3(0.2);
    proto::vec3 diffuse_color  = proto::vec3(0.8);
    proto::vec3 specular_color = proto::vec3(1.0);
    float alpha = 1.0;
    float shininess = 0.0;

    AssetHandle specular_map;
    AssetHandle diffuse_map;
    AssetHandle ambient_map;
    AssetHandle bump_map;
    AssetHandle opacity_map;
};
} //namespace proto 
