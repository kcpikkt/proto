#include "proto/core/platform/common.hh"
#if defined(PROTO_PLATFORM_LINUX)

#include "proto/core/platform/linux-clock.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/version.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/input.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/meta.hh"
#include "proto/core/context.hh"
#include "proto/core/io.hh"

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

void breakpoint() {};

using namespace proto;
using namespace proto::platform;
namespace proto { Context * context = nullptr; };

static PROTO_CLIENT_SETUP_FUNCTION_SIGNATURE
(proto_client_setup_stub) {
    puts("setup stub");
}
static PROTO_CLIENT_INIT_FUNCTION_SIGNATURE
(proto_client_init_stub) {
    puts("init stub");
}
static PROTO_CLIENT_UPDATE_FUNCTION_SIGNATURE
(proto_client_update_stub) {
    puts("update stub");
}

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
    static MouseEvent prev_mouse_event{};
    if(X_event.type == KeyPress || X_event.type == KeyRelease) {
        KeyEvent event;
        event.code =
            XkbKeycodeToKeysym(_context.display,
                               X_event.xkey.keycode, 0,
                               X_event.xkey.state & ShiftMask ? 1 : 0);

        _context.key_input_channel.emit(event);
    } else if(X_event.type == MotionNotify) {
        MouseEvent event;

                event.coord = proto::vec2(X_event.xmotion.x,
                                  X_event.xmotion.y);

        event.delta = prev_mouse_event.coord - event.coord;

        prev_mouse_event = event;

        if(!(X_event.xmotion.x == mouse_lock_pos.x &&
             X_event.xmotion.y == mouse_lock_pos.y)) {
            _context.mouse_input_channel.emit(event);
        }
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

int proto::platform::runtime([[maybe_unused]]int argc,[[maybe_unused]] char ** argv){

    proto::context = &_context;


    /***************************************************************
     * RUNTIME SETUP
     */

    // NOTE(kacper): arg parsing is the first thing that should happen right here,
    //               for now I just take the first arg as path to client lib
    assert(argc > 1);
    _context.clientlib_path = argv[1];

    // NOTE(kacper): here runtime first time opens dl to pass RuntimeSettings
    //               struct pointer to client_setup function, where client lib
    //               can setup some runtime startup settings.
    //               This happens only once, at runtime start so changes in
    //               client_setup (or PROTO_SETUP on client side), will not be
    //               reflected after hot-reload.
    ClientSetupFunction  * client_setup  = proto_client_setup_stub;

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

    client_setup(&settings);

    /***************************************************************
     * OBTAINING WINDOW & OPENGL INITIALIZATION
     */

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

    _context.window_size = proto::ivec2(960,1024);
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
                 KeyPressMask);

    static int visual_attribs[] =
        {GLX_RENDER_TYPE, GLX_RGBA_BIT,
         GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
         GLX_DOUBLEBUFFER, true,
         GLX_RED_SIZE, 1,
         GLX_GREEN_SIZE, 1,
         GLX_BLUE_SIZE, 1,
         GLX_DEPTH_SIZE, 3,
         None
        };

    int fbcount = 0;

    GLXFBConfig * fbc = glXChooseFBConfig(_context.display,
                                          DefaultScreen(_context.display),
                                          visual_attribs, &fbcount);
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


    XMapWindow(_context.display, _context.window);
    XStoreName(_context.display, _context.window, "proto!");
    glXMakeCurrent(_context.display, _context.window, gl_context);


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

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
        debug_error(proto::debug::category::main, "glewInit() failed.");

    glEnable(GL_BLEND);
    //glEnable(GL_MULTISAMPLE);  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, _context.window_size.x, _context.window_size.y);

    assert(!glGetError());

    //TODO(kacper): glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units)

    /***************************************************************
     * CONTEXT INITIALIZATION
     */
    // MEMORY
    namespace mem = proto::memory;

    u64 _mem_size = mem::gigabytes(2);
    void * _mem = malloc(_mem_size);
    assert(!_context.memory.init(_mem, _mem_size));

    _context.gp_string_allocator
        .init(&_context.memory, mem::megabytes(5));

    _context.gp_file_buffering_allocator
        .init(&_context.memory, mem::megabytes(100));

    _context.gp_texture_allocator
        .init(&_context.memory, mem::megabytes(100));

    _context.gp_debug_strings_allocator
        .init(&_context.memory, mem::megabytes(5));

    _context.asset_metadata_allocator
        .init(&_context.memory, mem::megabytes(50));

    _context.key_input_channel.init(100, &_context.memory);
    _context.mouse_input_channel.init(100, &_context.memory);

    // INITS
    // OpenGLContext
    set_debug_marker(_context.meshes, "context.texture_slots",
                     "local reflection of OpenGL texture units binding");
    _context.texture_slots.init_resize(32, &_context.memory);

    // AssetContext
    //set_debug_marker(_context.assets, "context.assets", "main asset registry");
    _context.assets.init(100, &_context.memory);

    set_debug_marker(_context.meshes, "context.meshes", "main mesh array");
    _context.meshes.init(100, &_context.memory);

    set_debug_marker(_context.materials, "context.materials",
        "main materials array (deprecate: meshes store their materials)");
    _context.materials.init(100, &_context.memory);

    set_debug_marker(_context.textures, "context.textures", "main texture array");
    _context.textures.init(100, &_context.memory);

    // Context

    if(settings.asset_paths &&
       settings.asset_paths.length())
    {
        _context.asset_paths
            .init_split(settings.asset_paths, ':', &_context.memory);
    } else {
        _context.asset_paths.init(&_context.memory);
        debug_warn(proto::debug::category::main,
                   "Asset search paths have not been set.");
    }

    struct { u8 ch[4]; } emergency_texture_data[] =
        {{ 255,   0, 255, 255}, { 255, 255, 255, 255},
         { 255, 255, 255, 255}, { 255,   0, 255, 255}};

    _context.default_ambient_map =
        create_asset("default_ambient_map", "",
                     AssetType<Texture>::index);
    _context.default_diffuse_map =
        create_asset("default_diffuse_map", "",
                     AssetType<Texture>::index);
    _context.default_specular_map =
        create_asset("default_specular_map", "",
                     AssetType<Texture>::index);

    if(!_context.default_ambient_map) 
        debug_warn(debug::category::main,
                   "Could not create default_ambient_map");

    if(!_context.default_diffuse_map) 
        debug_warn(debug::category::data,
                   "Could not create default_diffuse_map");

    if(!_context.default_specular_map) 
        debug_warn(debug::category::main,
                   "Could not create default_specular_map");

    Texture * default_ambient_texture =
        get_asset<Texture>(_context.default_ambient_map);
    assert(default_ambient_texture);
    default_ambient_texture->data = (void*)emergency_texture_data;
    default_ambient_texture->size = ivec2(2,2);
    default_ambient_texture->channels = 4;

    Texture * default_diffuse_texture =
        get_asset<Texture>(_context.default_diffuse_map);
    assert(default_diffuse_texture);
    default_diffuse_texture->data = (void*)emergency_texture_data;
    default_diffuse_texture->size = ivec2(2,2);
    default_diffuse_texture->channels = 4;

    Texture * default_specular_texture =
        get_asset<Texture>(_context.default_specular_map);
    assert(default_specular_texture);
    default_specular_texture->data = (void*)emergency_texture_data;
    default_specular_texture->size = ivec2(2,2);
    default_specular_texture->channels = 4;


    /*
    proto::memory::linear_allocator temp_allocator;
    temp_allocator.init(1024 * 5);

    struct option opts[] = {
                            {"game", 1, NULL, 'g'},
                            {0,0,0,0}
    };

    int opt_index = 0;
    int opt = 0;

    size_t cwd_path_len = fs::cwd_length() + 1;
    _context.cwd_path =
        (char*)temp_allocator.alloc(cwd_path_len, alignof(char));

    fs::cwd(_context.cwd_path, cwd_path_len);

    size_t gamelib_path_len = 0;

    while((opt = getopt_long(argc, argv, "g:", opts, &opt_index)) != -1) {
        switch(opt) {
        case 'g': {
                int gamelib_status_result = fs::status(optarg, nullptr);

                if(gamelib_status_result) {
                    printf("game file %s does not exist", optarg);
                } else {
                    gamelib_path_len = strlen(optarg) + 1;
                    _context.gamelib_path =
                        (char*)temp_allocator.alloc(gamelib_path_len, alignof(char));

                    strcpy(_context.gamelib_path, optarg);
                }
            } break;
            default:
                puts("help");
        }
    }

    size_t exe_path_len = cwd_path_len + strlen(argv[0]) + 1;
    _context.exe_path =
        (char*)temp_allocator.alloc(exe_path_len , alignof(char));

    strcpy(_context.exe_path, _context.cwd_path);
    strcat(_context.exe_path, "/");
    strcat(_context.exe_path, argv[0]);

    _context._allocator.init
        (gamelib_path_len + cwd_path_len + exe_path_len +
         1024);

    _context.exe_path =
        strcpy((char*) _context._allocator.alloc(exe_path_len,
                                                      alignof(char)),
               _context.exe_path);

    _context.cwd_path =
        strcpy((char*) _context._allocator.alloc(cwd_path_len,
                                                      alignof(char)),
               _context.cwd_path);

    if(gamelib_path_len != 0)
    _context.gamelib_path =
        strcpy((char*) _context._allocator.alloc(exe_path_len,
                                                      alignof(char)),
               _context.exe_path);

    temp_allocator.release();



    // puts(proto_banner);
    // puts(separator);

    printf("%sproto runtime, version %d.%d.%d.%d\n",
           indent,
           PROTO_VERSION_MAJOR,
           PROTO_VERSION_MINOR,
           PROTO_VERSION_BUILD,
           PROTO_VERSION_REVISION);
    puts(separator);

    size_t test_size = 288;
    proto::memory::LinkedListAllocator al;
    // proto::memory::LinkedListAllocator al2;
    // al2.init(test_size);
    al.init(test_size);
    auto pp = [&]() {
                printf("\n");
                for(int i = 0; i<24; i++) printf("%.2d ", i);
                printf("\n");
                for(size_t i = 0; i<24; i++) printf("---");
                printf("\n");

                for(size_t i = 0; i<test_size; i++) {
                    printf("%.2X ", *((unsigned char*)al.raw() + i));
                    if((i+1)%24 == 0) printf("\n");
                }
            };
    printf("header size %lu\n", sizeof(proto::memory::LinkedListAllocator::header));
    namespace mem = proto::memory;
    // unsigned char * p;
    // size_t sz = 32;
    // p = (unsigned char*)al.alloc(sz, 16); if(!p) assert(0);
    // for(size_t i=0; i<sz; i++) p[i] = 255;
    // unsigned char * h = (unsigned char*)al.get_header(al._first);
    // for(size_t i=0; i<sizeof(mem::LinkedListAllocator::header); i++) h[i] = i;


    // p = (unsigned char*)al.alloc(sz, 16); if(!p) assert(0);
    // for(size_t i=0; i<sz; i++) p[i] = 255;
    pp();
   */

    ClientSetContextFunction  * client_set_context = nullptr;
    ClientInitFunction   * client_init   = proto_client_init_stub;
    ClientUpdateFunction * client_update = proto_client_update_stub;

    //void * clientlib_h = dlopen(clientlib_path,
    //                             RTLD_LAZY);

    StringView clientlib_path = _context.clientlib_path;

    assert(clientlib_h);

    time_t clientlib_mtime;
    struct stat clientlib_statbuf;
    stat(clientlib_path, &clientlib_statbuf);
    // NOTE(kacper):
    // this is supposed to be #define st_mtime st_mtim.tv_sec for back compat
    // I dont need anthing more than more precise than seconds here
    clientlib_mtime = clientlib_statbuf.st_mtime;

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

    // live reload 
    int inotify_fd = inotify_init1(IN_NONBLOCK);
    assert(inotify_fd != -1);

    int clientlib_watch_fd = inotify_add_watch(inotify_fd,
                                                clientlib_path, 
                                                IN_MODIFY);

    //proto::context = &_context;
    client_set_context(&_context);
    assert(clientlib_watch_fd != -1);
    // read from fd

    //client_init();

    static_assert(proto::meta::is_integer<time_t>::value);

    struct timespec time_res;
    assert(!clock_getres(CLOCK_MONOTONIC_RAW, &time_res));

    proto::context = &_context;
    client_init();
    assert(!glGetError());

    _context.clock.init(60.0f);
    while(!_context.exit_sig) {

        _context.clock.tick();
        // temoporary code
        // TODO(kacper):
        // reimplement as a virtual platform api??
        stat(clientlib_path, &clientlib_statbuf);
        if(clientlib_mtime != clientlib_statbuf.st_mtime) {
            clientlib_mtime = clientlib_statbuf.st_mtime;
            // reloading code
            debug_info(proto::debug::category::main,
                       "client dynamic library reloaded");

        }
        //io::print(_context.count, "\n");

        //setup
        //init
        //link
        //update
        //unlink
        //close

        client_update();
        X_handle_events();
        glXSwapBuffers(_context.display, _context.window);
    }

    dlclose(clientlib_h);

    return 0;
}

#endif
