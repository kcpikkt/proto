#include "proto/proto.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/asset-system/serialization.hh"
#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/platform/File.hh" 
#include "proto/core/util/String.hh" 
#include "proto/core/reflection.hh" 

struct GLSLMaterialFieldRefl {
    const char * glsl_type;
    const char * glsl_name;
};


struct Mat {
    REFL_FIELDS(GLSLMaterialFieldRefl)((int) (albedo));
};


using namespace proto;


struct SokobanMap {
    enum MapElement {nothing, floor, wall, goal};

    ivec2 bounds;
    Array<MapElement> elements;
    Array<ivec2> crates;
    ivec2 player;
};

SokobanMap parse_map(StringView _textmap) {
    if(!_textmap.length())
        debug_warn(debug::category::main, "Empty map string");

    constexpr static char wall_char           = '#';
    constexpr static char player_char         = '@';
    constexpr static char player_on_goal_char = '+';
    constexpr static char crate_char          = '$';
    constexpr static char crate_on_goal_char  = '*';
    constexpr static char goal_char           = '.';
    constexpr static char floor_char          = '_';

    bool player_placed = false;

    SokobanMap map;
    map.crates.init(&context->memory);
    String textmap;
    textmap.init(_textmap, &context->memory);

    map.bounds = {0,0}; s32 line_len = 0;

    for(auto& c : textmap) {
        ivec2 current_pos = ivec2(line_len, map.bounds.y);

        if(c == ' ' || c == '-') c = floor_char;

        if(c == player_char || c == player_on_goal_char) {
            if(player_placed)
                debug_warn(debug::category::main,
                           "More than one player position on the map.");
            map.player = current_pos;
            player_placed = true;

            if(c == player_on_goal_char) c = goal_char;
            else c = floor_char;
        }

        if(c == crate_char || c == crate_on_goal_char) {
            map.crates.push_back(current_pos);

            if(c == crate_on_goal_char) c = goal_char;
            else c = floor_char;
        }

        if(c == '\n') {
            map.bounds = ivec2(max(map.bounds.x, line_len), map.bounds.y + 1);
            line_len = 0;
        } else
            line_len++;
    }

    if(textmap[_textmap.length() - 1] != '\n') map.bounds.y++;

    map.elements.init_resize(map.bounds.x * map.bounds.y, &context->memory);

    for(auto& e : map.elements) e = SokobanMap::nothing;

    u32 x,y; x=y=0;
    for(auto c : textmap) {
        s32 idx = x + y * map.bounds.x;
        if(c == wall_char)  map.elements[idx] = SokobanMap::wall;
        if(c == floor_char) map.elements[idx] = SokobanMap::floor;
        if(c == goal_char)  map.elements[idx] = SokobanMap::goal;

        x++;
        if(c == '\n') { y++; x = 0;}
    }
    return map;
}


PROTO_SETUP {
    settings->shader_paths = "src/demos/sokoban/shaders/";
}

AssetHandle mytex;
AssetHandle main_shader_h;

struct Test {
    Test() {}
    int a;
};

//template<typename T, >
//struct GLSLReflFieldMetadata {
//    const char * glsl_name;
//    const char * glsl_type;
//};

//struct Mat {
//    REFLECT(GLSLReflectMetadata) (
//                                  (AssetHandle) albedo, { "sampler2D", "albedo" }
//                                  (AssetHandle) metallic, { "sampler2D", "albedo" }
//                                  (AssetHandle) roughness, { "sampler2D", "albedo" }
//                                  (AssetHandle) cavity, { "sampler2D", "cavity" }
//                                  );
//};
#define REM(...) __VA_ARGS__
#define EAT(...)
#define TYPEOF(x) DETAIL_TYPEOF(DETAIL_TYPEOF_PROBE x,)
#define DETAIL_TYPEOF(...) DETAIL_TYPEOF_HEAD(__VA_ARGS__)
#define DETAIL_TYPEOF_HEAD(x, ...) REM x
#define DETAIL_TYPEOF_PROBE(...) (__VA_ARGS__),

u32 ubo;
PROTO_INIT {
    TYPEOF((int) a) a = 1;

    FOR_EACH(vardump, 1, 2, 8 );
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 152, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);


    if(!sys::is_file("outmesh/marble_albedo.past")) debug_error(0, "fucked up");
    mytex = ser::load_asset("outmesh/marble_albedo.past");
    gfx::gpu_upload(get_asset<Texture2D>(mytex));

    main_shader_h = 
        create_init_asset_rref<ShaderProgram>("sokoban_main")
            .$_attach_shader_file(ShaderType::Vert, "sokoban_main_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "sokoban_main_frag.glsl")
            .$_link().handle;

    if(!main_shader_h)
        debug_warn(debug::category::main, "Could not find main shader.");
}

PROTO_UPDATE {
    auto& ctx = *proto::context;
    auto time = ctx.clock.elapsed_time;

    //ctx.camera.rotation = glm::lookAt(vec3(0.0), vec3(0.0,0.0,-1.0), vec3(0.0,1.0,0.0));
    ctx.camera.position = vec3(0.0, 0.0, 10.0);

    SokobanMap map
        = parse_map("######\n"
                    "#____#\n"
                    "#___##\n"
                    "#__#_#\n"
                    "#_####\n"
                    "#___##\n"
                    "#____#\n"
                    "#____#\n"
                    "#____#\n"
                    "#____#\n"
                    "######\n"
                    );

    if(time > 1.0f)
        ctx.exit_sig = true;

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto& main_shader = get_asset_ref<ShaderProgram>(main_shader_h).$_use();

    vec2 tmp = ctx.window_size / map.bounds;
    float scr_element_size = min(tmp.x, tmp.y);
    glEnable(GL_DEPTH_TEST);

    mat4 mvp, model, view = ctx.camera.view(), projection = ctx.camera.projection();
    auto& cube = get_asset_ref<Mesh>(ctx.cube_h);

    main_shader
        .$_set_float ("u_time", time);

    #if 1 
    for(s32 y=0; y<map.bounds.y; y++) {
        for(s32 x=0; x<map.bounds.x; x++) {
            s32 idx = x + y * map.bounds.x;
            s32 z = -1;

            s32 texture_slot;
            /*  */ if(map.elements[idx] == SokobanMap::wall) {
                texture_slot = gfx::bind_texture(ctx.default_white_texture_h);
                z = 0;

            } else if(map.elements[idx] == SokobanMap::goal) {
                texture_slot = gfx::bind_texture(ctx.default_checkerboard_texture_h);

            } else {
                texture_slot = gfx::bind_texture(ctx.default_black_texture_h);
            }
            
            model = mat4(1.0);
            model = translate(model, vec3(x,y,z));
            mvp = projection * view * model;

            main_shader.$_set_mat4  ("u_mvp", &mvp);
                
            texture_slot = gfx::bind_texture(mytex);

            //gfx::render_mesh(&cube, true);
            gfx::render_texture_quad(texture_slot);
        }
    }
    #endif
    //gfx::render_texture_quad(gfx::bind_texture(mytex));

    gfx::stale_all_texture_slots();
}
