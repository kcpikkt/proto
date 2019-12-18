#pragma once
#include "proto/core/platform/macros.hh"
#if defined(PROTO_PLATFORM_LINUX)
#include <X11/keysym.h>
#endif


#include "proto/core/common/types.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/event-system.hh"
namespace proto {

const static char * ascii =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";


struct KeyState : Bitfield<u8> {
    constexpr static u8 pressed_bit = BIT(0);
};

struct KeyEvent {
    //    u32 device;
    //    u32 type;
    u32 code;
};

struct MouseMoveEvent {
    vec2 coord;
    vec2 delta;
};

struct MouseButtonEvent {
    vec2 coord;
    vec2 delta;
};


using KeyInputChannel = Channel<KeyEvent>;
using KeyInputSink = Sink<KeyEvent>;

using MouseMoveInputChannel = Channel<MouseMoveEvent>;
using MouseMoveInputSink = Sink<MouseMoveEvent>;

using MouseButtonInputChannel = Channel<MouseButtonEvent>;
using MouseButtonInputSink = Sink<MouseButtonEvent>;

namespace input {

namespace keycode {
#if defined(PROTO_PLATFORM_LINUX)

    constexpr static u32 home                 = XK_Home;
    constexpr static u32 left                 = XK_Left;
    constexpr static u32 up                   = XK_Up;
    constexpr static u32 right                = XK_Right;
    constexpr static u32 down                 = XK_Down;
    constexpr static u32 prior                = XK_Prior;
    constexpr static u32 page_up              = XK_Page_Up;
    constexpr static u32 next                 = XK_Next;
    constexpr static u32 page_down            = XK_Page_Down;
    constexpr static u32 end                  = XK_End;
    constexpr static u32 begin                = XK_Begin;
    constexpr static u32 space                = XK_space;
    constexpr static u32 exclam               = XK_exclam;
    constexpr static u32 quotedbl             = XK_quotedbl;
    constexpr static u32 numbersign           = XK_numbersign;
    constexpr static u32 dollar               = XK_dollar;
    constexpr static u32 percent              = XK_percent;
    constexpr static u32 ampersand            = XK_ampersand;
    constexpr static u32 apostrophe           = XK_apostrophe;
    constexpr static u32 quoteright           = XK_quoteright;
    constexpr static u32 parenleft            = XK_parenleft;
    constexpr static u32 parenright           = XK_parenright;
    constexpr static u32 asterisk             = XK_asterisk;
    constexpr static u32 plus                 = XK_plus;
    constexpr static u32 comma                = XK_comma;
    constexpr static u32 minus                = XK_minus;
    constexpr static u32 period               = XK_period;
    constexpr static u32 slash                = XK_slash;
    constexpr static u32 zero                 = XK_0;
    constexpr static u32 one                  = XK_1;
    constexpr static u32 two                  = XK_2;
    constexpr static u32 three                = XK_3;
    constexpr static u32 four                 = XK_4;
    constexpr static u32 five                 = XK_5;
    constexpr static u32 six                  = XK_6;
    constexpr static u32 seven                = XK_7;
    constexpr static u32 eight                = XK_8;
    constexpr static u32 nine                 = XK_9;
    constexpr static u32 colon                = XK_colon;
    constexpr static u32 semicolon            = XK_semicolon;
    constexpr static u32 less                 = XK_less;
    constexpr static u32 equal                = XK_equal;
    constexpr static u32 greater              = XK_greater;
    constexpr static u32 question             = XK_question;
    constexpr static u32 at                   = XK_at;
    constexpr static u32 A                    = XK_A;
    constexpr static u32 B                    = XK_B;
    constexpr static u32 C                    = XK_C;
    constexpr static u32 D                    = XK_D;
    constexpr static u32 E                    = XK_E;
    constexpr static u32 F                    = XK_F;
    constexpr static u32 G                    = XK_G;
    constexpr static u32 H                    = XK_H;
    constexpr static u32 I                    = XK_I;
    constexpr static u32 J                    = XK_J;
    constexpr static u32 K                    = XK_K;
    constexpr static u32 L                    = XK_L;
    constexpr static u32 M                    = XK_M;
    constexpr static u32 N                    = XK_N;
    constexpr static u32 O                    = XK_O;
    constexpr static u32 P                    = XK_P;
    constexpr static u32 Q                    = XK_Q;
    constexpr static u32 R                    = XK_R;
    constexpr static u32 S                    = XK_S;
    constexpr static u32 T                    = XK_T;
    constexpr static u32 U                    = XK_U;
    constexpr static u32 V                    = XK_V;
    constexpr static u32 W                    = XK_W;
    constexpr static u32 X                    = XK_X;
    constexpr static u32 Y                    = XK_Y;
    constexpr static u32 Z                    = XK_Z;
    constexpr static u32 bracketleft          = XK_bracketleft;
    constexpr static u32 backslash            = XK_backslash;
    constexpr static u32 bracketright         = XK_bracketright;
    constexpr static u32 asciicircum          = XK_asciicircum;
    constexpr static u32 underscore           = XK_underscore;
    constexpr static u32 grave                = XK_grave;
    constexpr static u32 quoteleft            = XK_quoteleft;
    constexpr static u32 a                    = XK_a;
    constexpr static u32 b                    = XK_b;
    constexpr static u32 c                    = XK_c;
    constexpr static u32 d                    = XK_d;
    constexpr static u32 e                    = XK_e;
    constexpr static u32 f                    = XK_f;
    constexpr static u32 g                    = XK_g;
    constexpr static u32 h                    = XK_h;
    constexpr static u32 i                    = XK_i;
    constexpr static u32 j                    = XK_j;
    constexpr static u32 k                    = XK_k;
    constexpr static u32 l                    = XK_l;
    constexpr static u32 m                    = XK_m;
    constexpr static u32 n                    = XK_n;
    constexpr static u32 o                    = XK_o;
    constexpr static u32 p                    = XK_p;
    constexpr static u32 q                    = XK_q;
    constexpr static u32 r                    = XK_r;
    constexpr static u32 s                    = XK_s;
    constexpr static u32 t                    = XK_t;
    constexpr static u32 u                    = XK_u;
    constexpr static u32 v                    = XK_v;
    constexpr static u32 w                    = XK_w;
    constexpr static u32 x                    = XK_x;
    constexpr static u32 y                    = XK_y;
    constexpr static u32 z                    = XK_z;
    constexpr static u32 braceleft            = XK_braceleft;
    constexpr static u32 bar                  = XK_bar;
    constexpr static u32 braceright           = XK_braceright;
    constexpr static u32 asciitilde           = XK_asciitilde;
    constexpr static u32 left_shift           = XK_Shift_L;
    constexpr static u32 right_shift          = XK_Shift_R;
    constexpr static u32 left_control         = XK_Control_L;
    constexpr static u32 right_control        = XK_Control_R;
    constexpr static u32 caps_lock            = XK_Caps_Lock;
    constexpr static u32 backspace            = XK_BackSpace;
    constexpr static u32 Tab                  = XK_Tab;
    constexpr static u32 Linefeed             = XK_Linefeed;
    constexpr static u32 clear                = XK_Clear;
    //constexpr static u32 return               = XK_Return;
    constexpr static u32 pause                = XK_Pause;
    constexpr static u32 scroll_lock          = XK_Scroll_Lock;
    constexpr static u32 sys_req              = XK_Sys_Req;
    constexpr static u32 escape               = XK_Escape;
    //constexpr static u32 delete               = XK_Delete;
       


#endif
}

}
}
