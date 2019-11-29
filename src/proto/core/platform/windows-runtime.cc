#include "proto/core/platform/common.hh"
#if defined(PROTO_PLATFORM_WINDOWS)

proto::platform::RuntimeState
proto::platform::runtime_state = proto::platform::RuntimeState{};

#include "proto/core/platform/api.hh"
#include "proto/core/version.hh"
#include "proto/core/memory/linear_allocator.hh"
#include "proto/core/memory/linked_list_allocator.hh"

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <gl/gl.h>
#include <gl/wglext.h>

static PROTO_CLIENT_SETUP_FUNCTION_SIGNATURE
(proto_client_setup_stub) {
    puts("update stub");
}
static PROTO_CLIENT_INIT_FUNCTION_SIGNATURE
(proto_client_init_stub) {
    puts("init stub");
}
static PROTO_CLIENT_UPDATE_FUNCTION_SIGNATURE
(proto_client_update_stub) {
    puts("update stub");
}

using namespace proto;
using namespace proto::platform;

LRESULT CALLBACK window_procedure(HWND window_h, UINT message,
                                  WPARAM w_param, LPARAM l_param)
{
    switch(message){
    case WM_KEYDOWN: {} break;
        if(w_param == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        break;
    case WM_CLOSE: {
        PostQuitMessage(0);
        break;
    } break;
    default:
        return DefWindowProc(window_h, message, w_param, l_param);
    }
    return 0;
}

int proto::platform::runtime(int argc, char ** argv){
    // Setting up the console
    BOOL launched_from_console = AttachConsole(ATTACH_PARENT_PROCESS);
    if(launched_from_console == 0) {
        BOOL new_console = AllocConsole();
        assert(new_console);
    }
    runtime_state.stdin_h = GetStdHandle(STD_INPUT_HANDLE);
    assert(runtime_state.stdin_h != INVALID_HANDLE_VALUE);

    runtime_state.stdout_h = GetStdHandle(STD_OUTPUT_HANDLE);
    assert(runtime_state.stdout_h != INVALID_HANDLE_VALUE);

    runtime_state.stderr_h = GetStdHandle(STD_ERROR_HANDLE);
    assert(runtime_state.stderr_h != INVALID_HANDLE_VALUE);

    int utf8_code_page = 65001;
    SetConsoleCP(utf8_code_page);
    SetConsoleOutputCP(utf8_code_page);

    proto::platform::print_f("\n");


    WNDCLASSEX window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = window_procedure;
    window_class.hInstance = runtime_state.instance_h;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.lpszClassName = "Core";

    ATOM register_class_result = RegisterClassEx(&window_class);
    
    assert(register_class_result != 0);

    runtime_state.window_h = CreateWindowEx(WS_EX_CLIENTEDGE,
                                   "Core", "Fake Window",
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   512,512,NULL,NULL,
                                   runtime_state.instance_h, NULL);

    HDC device_context_h = GetDC(runtime_state.window_h);

    PIXELFORMATDESCRIPTOR pixel_format_desc;
    ZeroMemory(&pixel_format_desc, sizeof(pixel_format_desc));
    pixel_format_desc.nSize = sizeof(pixel_format_desc);
    pixel_format_desc.nVersion = 1;
    pixel_format_desc.dwFlags = (PFD_DRAW_TO_WINDOW |
                                           PFD_SUPPORT_OPENGL |
                                           PFD_DOUBLEBUFFER    );
    pixel_format_desc.iPixelType = PFD_TYPE_RGBA;
    pixel_format_desc.cColorBits = 32;
    pixel_format_desc.cAlphaBits = 8;
    pixel_format_desc.cDepthBits = 24;

    int pixel_format_desc_index = ChoosePixelFormat(device_context_h,
                                                    &pixel_format_desc);
    assert(pixel_format_desc_index != 0);
    
    assert( SetPixelFormat(device_context_h,
                           pixel_format_desc_index,
                           &pixel_format_desc) != 0);

    HGLRC gl_rendering_context_h = wglCreateContext(device_context_h);

    assert(gl_rendering_context_h);

    assert(wglMakeCurrent(device_context_h, gl_rendering_context_h));

    ShowWindow(runtime_state.window_h, argc);

    glClearColor(0.0f,0.0,1.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SwapBuffers(device_context_h);

    /*
      NOTE(kacper): you need to create window two times in order 
        to have advanced pixel format attribs but do you need them??
    using PfnwGLChoosePixelFormatARBProc = PFNWGLCHOOSEPIXELFORMATARBPROC;
    PfnwGLChoosePixelFormatARBProc wglChoosePixelFormatARB = nullptr;
    wglChoosePixelFormatARB =
        reinterpret_cast<PfnwGLChoosePixelFormatARBProc>
        (wglGetProcAddress("wglChoosePixelFormatARB"));
    assert(wglChoosePixelFormatARB);

    using PfnwGLCreateContextAttribsARBProc = PFNWGLCREATECONTEXTATTRIBSARBPROC;
    PfnwGLCreateContextAttribsARBProc wglCreateContextAttribsARB = nullptr;
    wglCreateContextAttribsARB =
        reinterpret_cast<PfnwGLCreateContextAttribsARBProc>
        (wglGetProcAddress("wglCreateContextAttribsARB"));
    assert(wglCreateContextAttribsARB);
    */


    //PFNWGLCHOOSEPIXELFORMATARBPROC wgl_choose_pixel_format_ARB = nullptr;
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
    runtime_state.cwd_path =
        (char*)temp_allocator.alloc(cwd_path_len, alignof(char));

    fs::cwd(runtime_state.cwd_path, cwd_path_len);

    size_t client_lib_path_len = 0;

    while((opt = getopt_long(argc, argv, "g:", opts, &opt_index)) != -1) {
        switch(opt) {
        case 'g': {
                int client_lib_status_result = fs::status(optarg, nullptr);

                if(client_lib_status_result) {
                    printf("game file %s does not exist", optarg);
                } else {
                    client_lib_path_len = strlen(optarg) + 1;
                    runtime_state.client_lib_path =
                        (char*)temp_allocator.alloc(client_lib_path_len, alignof(char));

                    strcpy(runtime_state.client_lib_path, optarg);
                }
            } break;
            default:
                puts("help");
        }
    }

    size_t exe_path_len = cwd_path_len + strlen(argv[0]) + 1;
    runtime_state.exe_path =
        (char*)temp_allocator.alloc(exe_path_len , alignof(char));

    strcpy(runtime_state.exe_path, runtime_state.cwd_path);
    strcat(runtime_state.exe_path, "/");
    strcat(runtime_state.exe_path, argv[0]);

    runtime_state._allocator.init
        (client_lib_path_len + cwd_path_len + exe_path_len +
         1024);

    runtime_state.exe_path =
        strcpy((char*) runtime_state._allocator.alloc(exe_path_len,
                                                      alignof(char)),
               runtime_state.exe_path);

    runtime_state.cwd_path =
        strcpy((char*) runtime_state._allocator.alloc(cwd_path_len,
                                                      alignof(char)),
               runtime_state.cwd_path);

    if(client_lib_path_len != 0)
    runtime_state.client_lib_path =
        strcpy((char*) runtime_state._allocator.alloc(exe_path_len,
                                                      alignof(char)),
               runtime_state.exe_path);

    temp_allocator.release();
    */

    ClientSetupFunction  * client_setup  = proto_client_setup_stub;
    ClientInitFunction   * client_init   = proto_client_init_stub;
    ClientUpdateFunction * client_update = proto_client_update_stub;

    
    HMODULE client_lib = LoadLibrary("..\src\demos\test\test.dll");

    // proto::platform::print_f("%sproto runtime, version %d.%d.%d.%d\n",
    //        indent,
    //        PROTO_VERSION_MAJOR,
    //        PROTO_VERSION_MINOR,
    //        PROTO_VERSION_BUILD,
    //        PROTO_VERSION_REVISION);
    // proto::platform::print_f(separator);

    Sleep(400);
    size_t test_size = 288;
    // proto::memory::linked_list_allocator al;
    // // proto::memory::linked_list_allocator al2;
    // // al2.init(test_size);
    // al.init(test_size);
    // printf("header size %lu\n", sizeof(proto::memory::linked_list_allocator::header));
    // namespace mem = proto::memory;
    // pp();


    client_setup();
    client_init();
    client_update();

    return 0;
}

#endif
