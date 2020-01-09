#include "proto/core/platform/common.hh"
#if defined(PROTO_PLATFORM_LINUX)

#include "proto/core/platform/OSAllocator.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/graphics/primitives.hh"
#include "proto/core/graphics/gl.hh"
#include "proto/core/version.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/input.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/meta.hh"
#include "proto/core/math/random.hh"
#include "proto/core/context.hh"
#include "proto/core/io.hh"
#include "proto/core/util/defer.hh"
#include "proto/core/entity-system.hh"

#include <sys/types.h>
#include <stdio.h>
#include <getopt.h>
#include <dlfcn.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include <unistd.h>
#include <tgmath.h>
#include <signal.h>
#include <assert.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

// switches
#define DEFAULT_TEXTURES 0
#define DEFAULT_SHADERS 0
#define DEFAULT_MESHES 1

void breakpoint() {};

using namespace proto;
using namespace proto::platform;
namespace proto { Context * context = nullptr; };

static PROTO_CLIENT_SETUP_FUNCTION_SIGNATURE  (proto_client_setup_stub)  {}
static PROTO_CLIENT_INIT_FUNCTION_SIGNATURE   (proto_client_init_stub)   {}
static PROTO_CLIENT_UPDATE_FUNCTION_SIGNATURE (proto_client_update_stub) {
    log_info(debug::category::main, "No update procedure provided, exiting.");
    proto::context->exit_sig = true;
}
static PROTO_CLIENT_LINK_FUNCTION_SIGNATURE   (proto_client_link_stub)   {}
static PROTO_CLIENT_UNLINK_FUNCTION_SIGNATURE (proto_client_unlink_stub) {}
static PROTO_CLIENT_CLOSE_FUNCTION_SIGNATURE  (proto_client_close_stub)  {}

//namespace proto { Context * context = nullptr; }

proto::Context _context = proto::Context{};

// -Wpedantic official workaround
#define DLSYM_ASSIGN(TO, FROM)              \
    ((*(void **) (&TO)) = FROM)

namespace proto {
namespace platform {
    int runtime(int argc, char ** argv);
}
}

proto::ivec2 mouse_lock_pos = proto::ivec2(200,200);
static void X_event_callback(XEvent& X_event) {
    static MouseMoveEvent prev_mouse_move_event{};

    if(X_event.type == KeyPress || X_event.type == KeyRelease) {
        KeyEvent event;

        event.code =
            XkbKeycodeToKeysym(_context.display,
                               X_event.xkey.keycode, 0,
                               X_event.xkey.state & ShiftMask ? 1 : 0);

        _context.key_input_channel.emit(event);
    } else if(X_event.type == MotionNotify) {
        MouseMoveEvent event;
        event.coord = proto::vec2(X_event.xmotion.x,
                                  X_event.xmotion.y);

        event.delta = prev_mouse_move_event.coord - event.coord;

        prev_mouse_move_event = event;

        if(!(X_event.xmotion.x == mouse_lock_pos.x &&
             X_event.xmotion.y == mouse_lock_pos.y)) {
            _context.mouse_move_input_channel.emit(event);
        }
    } else if (X_event.type == ButtonPress || X_event.type == ButtonRelease) {
        MouseButtonEvent event;
        event.coord = proto::vec2(X_event.xbutton.x,
                                  X_event.xbutton.y);
        _context.mouse_button_input_channel.emit(event);
    } else if (X_event.type == ConfigureNotify) {

        if(_context.window_size.x != X_event.xconfigure.width &&
           _context.window_size.y != X_event.xconfigure.height) {

            //_context.window_size.x = X_event.xconfigure.width;
            //_context.window_size.y = X_event.xconfigure.height;
            //debug_info(1, mouse_lock_pos.x, " ", mouse_lock_pos.x);

            //mouse_lock_pos = _context.window_size/2;
        }
    }
}

static void X_handle_events() {
    XQueryKeymap(_context.display, _context.key_state);

    XEvent ev;
    while(XPending(_context.display)) {
        XNextEvent(_context.display, &ev);
        X_event_callback(ev);
    }
    XWarpPointer(_context.display,
                 _context.window,
                 _context.window,
                 0,0,0,0,
                 mouse_lock_pos.x,
                 mouse_lock_pos.y);
}

typedef GLXContext (*glXCreateContextAttribsARBProc)
(Display*, GLXFBConfig, GLXContext, Bool, const int*);

typedef GLXFBConfig (*glXChooseFBConfigProc)
(Display, int, const int, int);

RuntimeSettings settings;
void * clientlib_h;

int proto::platform::runtime(int argc, char ** argv){


    proto::context = &_context;
    auto& ctx = _context;

    ctx.argc = argc; ctx.argv = argv;
    namespace mem = proto::memory;

    // just for cmdline args
    /***************************************************************
     * RUNTIME SETUP
     */

    // NOTE(kacper): arg parsing is the first thing that should happen right here,
    //               for now I just take the first arg as path to client lib
    assert(argc > 1);
    _context.clientlib_path = argv[1];

    //_context.cmdline.init(argc, &tmp_presetup_memory);
    //debug_info(1, ctx.cmdline.size());

    //for(s32 i=0; i<argc; i++) _context.cmdline.store(argv[i]);

    // NOTE(kacper): here runtime first time opens dl to pass RuntimeSettings
    //               struct pointer to client_setup function, where client lib
    //               can setup some runtime startup settings.
    //               This happens only once, at runtime start so changes in
    //               client_setup (or PROTO_SETUP on client side), will not be
    //               reflected after hot-reload.
    ClientSetupFunction  * client_setup  = proto_client_setup_stub;

    assert(_context.clientlib_path.is_cstring());
    clientlib_h = dlopen(_context.clientlib_path, RTLD_LAZY);

    const char * dlerr = dlerror();
    assert_info(!dlerr,
                proto::debug::category::main,
                "Error occured when opening client dynamic library ",
                _context.clientlib_path, ": ", dlerr);

    void * maybe_client_setup =
        dlsym(clientlib_h, PROTO_CLIENT_SETUP_FUNCTION_NAME);
    if(maybe_client_setup)
       DLSYM_ASSIGN( client_setup, maybe_client_setup );
    else 
        debug_warn(debug::category::main,
                   "no setup function found in client library.");

    client_setup(&settings);

    /***************************************************************
     * OBTAINING WINDOW & OPENGL INITIALIZATION
     */
    //bool window_mode = settings.mode.check(RuntimeSettings::window_mode_bit);
    //bool opengl_mode = settings.mode.check(RuntimeSettings::opengl_mode_bit);
    //meh, 
    //assert(!(window_mode xor opengl_mode));

        // NOTE(kacper):
        // IMO I should not have to get addresses of those functions
        // manually since they are GL 1.3, am I right?
        // Though, even if those function pointers were already bound,
        // it should still work so this only adds redundancy.
    glXChooseFBConfig =
        (PFNGLXCHOOSEFBCONFIGPROC) 
        glXGetProcAddress((const GLubyte*)"glXChooseFBConfig");

    glXGetVisualFromFBConfig =
        (PFNGLXGETVISUALFROMFBCONFIGPROC) 
        glXGetProcAddress((const GLubyte*)"glXGetVisualFromFBConfig");

    _context.display = XOpenDisplay(NULL);

    assert(_context.display);

    int glx_major, glx_minor;
    assert_info((glXQueryVersion( _context.display, &glx_major, &glx_minor ) && 
                glx_major == 1 && glx_minor > 2) ,
                proto::debug::category::main,
                "Invalid GLX version");

    // So every linux window manager is actually just a root X window?
    Window root = DefaultRootWindow(_context.display);

    _context.window_size = proto::ivec2(512,512);
    mouse_lock_pos = _context.window_size/2;

    _context.window =
        XCreateSimpleWindow(_context.display, root,
                            10, 10,
                            _context.window_size.x,
                            _context.window_size.y, 0, 0, 0);

    assert(_context.window);
    XSelectInput(_context.display,
                _context.window,
                StructureNotifyMask |
                PointerMotionMask |
                ExposureMask |
                KeyPressMask |
                KeyReleaseMask |
                ButtonPressMask);

    static int visual_attribs[] =
        {GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, true,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_DEPTH_SIZE, 3,
         // deffered rendering, no multisampling unfortunatelly
         //GLX_SAMPLE_BUFFERS  , 1, // MSAA
         //GLX_SAMPLES         , 4, // MSAA
        None
        };

    int fbcount = 0;

    GLXFBConfig * fbc = glXChooseFBConfig(_context.display,
                                        DefaultScreen(_context.display),
                                        visual_attribs, &fbcount);
    assert(fbc);
    // so visual is a bit like winapi pixel format, isn't it?
    XVisualInfo * visual = glXGetVisualFromFBConfig(_context.display, fbc[0]); 
    assert(visual);

    GLXContext tmp_gl_context =
        glXCreateContext(_context.display, visual, NULL, GL_TRUE);


    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
        (glXCreateContextAttribsARBProc)
        glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");

    assert(glXCreateContextAttribsARB);

    glXDestroyContext(_context.display, tmp_gl_context);

    static int context_attribs[] =
        {GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        None };

    GLXContext gl_context =
        glXCreateContextAttribsARB(_context.display, fbc[0], NULL, true,
                                context_attribs);

    assert(gl_context);

    assert_info(fbc,
                proto::debug::category::graphics,
                "Could not match any framebuffer configuration");


    if(settings.mode.check(RuntimeSettings::window_mode_bit))
        XMapWindow(_context.display, _context.window);

    XStoreName(_context.display, _context.window, "proto!");
    glXMakeCurrent(_context.display, _context.window, gl_context);

    assert(!glGetError());

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
        debug_error(proto::debug::category::main, "glewInit() failed.");
    

    log_info(proto::debug::category::main, separator);
    log_info(proto::debug::category::main, indent,
            "proto framework v",
            PROTO_VERSION_MAJOR , ".",
            PROTO_VERSION_MINOR , ".",
            PROTO_VERSION_BUILD , ".",
            PROTO_VERSION_REVISION);

    log_info(proto::debug::category::main, indent,
            "OpenGL " ,glGetString(GL_VERSION),
            ", GLSL " ,glGetString(GL_SHADING_LANGUAGE_VERSION));

    log_info(proto::debug::category::main, indent);

    log_info(proto::debug::category::main, indent,
            "client: ", _context.clientlib_path);

    log_info(proto::debug::category::main, separator);

    //TODO(kacper): glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units)

    /***************************************************************
     * CONTEXT INITIALIZATION
     */
    // MEMORY

    u64 _mem_size = mem::gigabytes(2);
    void * _mem = malloc(_mem_size);
    assert(!_context.memory.init(_mem, _mem_size));

    //_context.gp_string_allocator
    //    .init(&_context.memory, mem::megabytes(5));

    //_context.gp_file_buffering_allocator
    //    .init(&_context.memory, mem::megabytes(100));

    //_context.gp_texture_allocator
    //    .init(&_context.memory, mem::megabytes(100));

    _context.gp_debug_strings_allocator
        .init(&_context.memory, mem::megabytes(5));

    _context.asset_metadata_allocator
        .init(&_context.memory, mem::megabytes(50));

    _context.key_input_channel.init(10, &_context.memory);
    _context.mouse_move_input_channel.init(10, &_context.memory);
    _context.mouse_button_input_channel.init(10, &_context.memory);

    // INITS
    // OpenGLContext
    set_debug_marker(_context.textures, "context.texture_slots",
                    "local reflection of OpenGL texture units binding");
    // TODO(kcpikkt): get number of slots from openGL
    _context.texture_slots.init_resize(32, &_context.memory);
    _context.texture_slots_index.init(32);
    _context.texture_slots.set_autodestruct();

    // AssetContext
    //set_debug_marker(_context.assets, "context.assets", "main asset registry");
    //_context.assets.init(100, &_context.memory);

    //set_debug_marker(_context.meshes, "context.meshes", "main mesh array");
    _context.meshes.init(100, &_context.memory);
    _context.meshes.set_autodestruct(); 

    //set_debug_marker(_context.materials, "context.materials",
    //    "main materials array (deprecate: meshes store their materials)");
    _context.materials.init(100, &_context.memory);
    _context.materials.set_autodestruct(); 

    //set_debug_marker(_context.textures, "context.textures", "main texture array");
    _context.textures.init(100, &_context.memory); 
    _context.textures.set_autodestruct(); 

    //set_debug_marker(_context.cubemaps, "context.cubemaps", "main cubemaps array");
    _context.cubemaps.init(10, &_context.memory);
    _context.cubemaps.set_autodestruct(); 

    //set_debug_marker(_context.cubemaps, "context.shader_programs",
    //                 "main shader_programs array");
    _context.shader_programs.init(10, &_context.memory);
    _context.shader_programs.set_autodestruct();

    //set_debug_marker(_context.textures, "context.framebuffers",
    //                 "main framebufffer array");
    _context.framebuffers.init(0, &_context.memory);
    _context.framebuffers.set_autodestruct();

    _context.entities.init(10, &_context.memory);
    _context.entities.set_autodestruct();

    _context.comp.transform.init(10, &_context.memory);
    _context.comp.transform.set_autodestruct();

    _context.comp.render_mesh.init(10, &_context.memory);
    _context.comp.render_mesh.set_autodestruct();

    _context.comp.pointlights.init(10, &_context.memory);
    _context.comp.pointlights.set_autodestruct();

    // Context

    if(settings.asset_paths && settings.asset_paths.length()) {
        _context.asset_paths
            .init_split(settings.asset_paths, ':', &_context.memory);
    } else
        _context.asset_paths.init(&_context.memory);

    _context.asset_paths.set_autodestruct();

    if(settings.shader_paths && settings.shader_paths.length())
        _context.shader_paths
            .init_split(settings.shader_paths, ':', &_context.memory);
    else 
        _context.shader_paths.init(&_context.memory);

    _context.shader_paths.store("src/proto/shaders");
    _context.shader_paths.set_autodestruct();

    // DEFAULT TEXTURES

#if DEFAULT_TEXTURES
    struct { u8 ch[4]; } default_checkerboard_texture_data[] =
        {{ 255,   0, 255, 255}, { 255, 255, 255, 255},
         { 255, 255, 255, 255}, { 255,   0, 255, 255}};

    struct { u8 ch[4]; } default_white_texture_data[] = {{ 255, 255, 255, 255}};
    struct { u8 ch[4]; } default_black_texture_data[] = {{   0,   0,   0, 255}};

    Texture2D & default_white_texture =
        create_asset_rref<Texture2D>("defualt_white_texture");

    default_white_texture.init(ivec2(1,1), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    default_white_texture.data = (void*)default_white_texture_data;
    graphics::gpu_upload(&default_white_texture);
    assert(!glGetError());
    _context.default_white_texture_h = default_white_texture.handle;

    Texture2D & default_black_texture =
        create_asset_rref<Texture2D>("defualt_black_texture");

    default_black_texture.init(ivec2(1,1), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    default_black_texture.data = (void*)default_black_texture_data;
    graphics::gpu_upload(&default_black_texture);
    _context.default_black_texture_h = default_black_texture.handle;

    Texture2D & default_checkerboard_texture =
        create_asset_rref<Texture2D>("defualt_checkerboard_texture");

    default_checkerboard_texture.init(ivec2(2,2), GL_RGBA8, GL_RGBA);
    default_checkerboard_texture.data = (void*)default_checkerboard_texture_data;
    graphics::gpu_upload(&default_checkerboard_texture);
    _context.default_checkerboard_texture_h = default_checkerboard_texture.handle;
#endif


#if DEFAULT_MESHES
    Mesh & cube = create_asset<Mesh, Mesh&>("default_cube");
    cube.flags = Mesh::cached_bit;
    cube.cached = cube_vertices;
    cube.vertex_count = 36;
    cube.index_count = 0;

    _context.cube_h = cube.handle;
    //graphics::gpu_upload(&cube);

    //Mesh & quad = create_init_asset_rref<Mesh>("default_quad");
    //quad.vertices.resize(4);
    //assert(sizeof(quad_vertices) == sizeof(Vertex) * 4);
    //memcpy(quad.vertices._data, quad_vertices, sizeof(Vertex) * 4);
    //graphics::gpu_upload(&quad);
    //_context.quad_h = quad.handle;

    //Mesh & std_basis = create_init_asset_rref<Mesh>("default_std_basis");
    //std_basis.vertices.resize(6);
    //assert(sizeof(std_basis_vertices) == sizeof(Vertex) * 6);
    //memcpy(std_basis.vertices._data, std_basis_vertices, sizeof(Vertex) * 6);
    //graphics::gpu_upload(&std_basis);
    //_context.std_basis_h = std_basis.handle;
#endif


#if DEFAULT_SHADERS

    auto& quad_shader =
        create_init_asset_rref<ShaderProgram>("default_quad_shader");
    quad_shader.attach_shader_file(ShaderType::Vert, "quad_vert.glsl");
    quad_shader.attach_shader_file(ShaderType::Frag, "quad_frag.glsl");
    quad_shader.link();
    _context.quad_shader_h = quad_shader.handle;

    auto& std_basis_shader =
        create_init_asset_rref<ShaderProgram>("default_std_basis_shader");
    std_basis_shader.attach_shader_file(ShaderType::Vert, "pass_mvp_vert.glsl");
    std_basis_shader.attach_shader_file(ShaderType::Frag, "std_basis_frag.glsl");
    std_basis_shader.link();
    _context.std_basis_shader_h = std_basis_shader.handle;

    auto& gbuffer_shader =
        create_init_asset_rref<ShaderProgram>("default_gbuffer_shader");
    gbuffer_shader.attach_shader_file(ShaderType::Vert, "g-buffer_vert.glsl");
    gbuffer_shader.attach_shader_file(ShaderType::Frag, "g-buffer_frag.glsl");
    gbuffer_shader.link();
    _context.gbuffer_shader_h = gbuffer_shader.handle;

    
    _context.skybox_shader_h =
        create_init_asset_rref<ShaderProgram>("default_skybox_shader")
            .$_attach_shader_file(ShaderType::Vert, "pass_mvp_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "skybox_frag.glsl")
            .$_link().handle;
#endif

    #if 0
    _context.default_framebuffer = &_context.framebuffers.push_back();
    _context.default_framebuffer->FBO = 0;
    _context.default_framebuffer->size = _context.window_size;
    _context.default_framebuffer->color_attachments.init(&_context.memory);

    _context.current_read_framebuffer = _context.default_framebuffer;
    _context.current_draw_framebuffer = _context.default_framebuffer;
    #endif

    random::seed_mt64(settings.mt64_seed);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_MULTISAMPLE);  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, _context.window_size.x, _context.window_size.y);
    

    //if((_context.inotify_fd = inotify_init1(IN_NONBLOCK)) == -1) {
    //    debug_error(debug::category::main,
    //                "inotify_init1() failed.");
    //}
    //_context.watched_files.init(3, &_context.memory);

    ClientSetContextFunction * client_set_context = nullptr;
    ClientInitFunction *   client_init   = proto_client_init_stub;
    ClientUpdateFunction * client_update = proto_client_update_stub;
    ClientLinkFunction *   client_link   = proto_client_link_stub;
    ClientUnlinkFunction * client_unlink = proto_client_unlink_stub;
    ClientCloseFunction *  client_close  = proto_client_close_stub;

    assert(clientlib_h);

    static_assert(proto::meta::is_integer<time_t>::value);

    assert(_context.clientlib_path.is_cstring());

    struct stat clientlib_statbuf;
    stat(_context.clientlib_path, &clientlib_statbuf);

    decltype(meta::declval<timespec>().tv_sec)
        clientlib_mtime = clientlib_statbuf.st_mtime;

    auto clientlib_bind_procs =
        [&](){
            void * maybe_set_context =
                dlsym(clientlib_h, PROTO_CLIENT_SET_CONTEXT_FUNCTION_NAME);

            assert_info(maybe_set_context,
                        proto::debug::category::main,
                        "Could not locate "
                        PROTO_CLIENT_SET_CONTEXT_FUNCTION_NAME
                        " in client library. "
                        "Are you sure you included proto/proto.hh?");

            DLSYM_ASSIGN( client_set_context, maybe_set_context );

            void * maybe_client_init =
                dlsym(clientlib_h, PROTO_CLIENT_INIT_FUNCTION_NAME);
            if(maybe_client_init)
                DLSYM_ASSIGN( client_init, maybe_client_init );

            void * maybe_client_update =
                dlsym(clientlib_h, PROTO_CLIENT_UPDATE_FUNCTION_NAME);
            if(maybe_client_update)
                DLSYM_ASSIGN( client_update, maybe_client_update );

            void * maybe_client_link =
                dlsym(clientlib_h, PROTO_CLIENT_LINK_FUNCTION_NAME);
            if(maybe_client_link)
                DLSYM_ASSIGN( client_link, maybe_client_link );

            void * maybe_client_unlink =
                dlsym(clientlib_h, PROTO_CLIENT_UNLINK_FUNCTION_NAME);
            if(maybe_client_unlink)
                DLSYM_ASSIGN( client_unlink, maybe_client_unlink );

            void * maybe_client_close =
                dlsym(clientlib_h, PROTO_CLIENT_CLOSE_FUNCTION_NAME);
            if(maybe_client_close)
                DLSYM_ASSIGN( client_close, maybe_client_close );
        };

    clientlib_bind_procs();

    client_set_context(&_context);

    struct timespec time_res;
    assert(!clock_getres(CLOCK_MONOTONIC_RAW, &time_res));

    client_init();
    assert(!glGetError());

    _context.clock.init(30.0f);
    while(!_context.exit_sig) {
        _context.clock.tick();

        stat(_context.clientlib_path, &clientlib_statbuf);
        if(clientlib_mtime != clientlib_statbuf.st_mtime) {
            clientlib_mtime = clientlib_statbuf.st_mtime;

            debug_info(proto::debug::category::main,
                       "client library changed on disk, attempting hot-swap.");

            assert(clientlib_h);

            void * client_preserve = nullptr;
            client_unlink();
            dlclose(clientlib_h);

            clientlib_h = dlopen(_context.clientlib_path, RTLD_LAZY);
            client_set_context(&_context);
            client_link();

            const char * dlerr = dlerror();
            assert_info(!dlerr,
                        proto::debug::category::main,
                        "Error occured when opening client dynamic library ",
                        _context.clientlib_path, ": ", dlerr);

            clientlib_bind_procs();

            // reloading code
            debug_info(proto::debug::category::main,
                       "client dynamic library reloaded");
        }

        client_update();

        X_handle_events();
        glXSwapBuffers(_context.display, _context.window);
    }

    client_close();
    dlclose(clientlib_h);

    dlerr = dlerror();
    assert_info(!dlerr,
                proto::debug::category::main,
                "Error occured when closing client dynamic library ",
                _context.clientlib_path, ": ", dlerr);
    return 0;
}

#endif
