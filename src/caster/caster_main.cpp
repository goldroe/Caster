#pragma warning(push)
#pragma warning( disable : 4244)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include <ft2build.h>
#include <freetype/freetype.h>

#include <simdjson\simdjson.h>
#include <simdjson\simdjson.cpp>

#include "base/base_core.h"
#include "base/base_arena.h"
#include "base/base_math.h"
#include "base/base_strings.h"
#include "auto_array.h"
#include "os/os.h"
#include "path/path.h"

#include "render/render_core.h"
#include "render/d3d11/render_d3d11.h"

#include "font/font.h"
#include "draw/draw.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_DECORATE(name) base_##name
#include <stb_sprintf.h>

#include "base/base_core.cpp"
#include "base/base_arena.cpp"
#include "base/base_math.cpp"
#include "base/base_strings.cpp"
#include "os/os.cpp"
#include "path/path.cpp"
#include "render/render_core.cpp"
#include "render/d3d11/render_d3d11.cpp"
#include "font/font.cpp"
#include "draw/draw.cpp"

#include "input.h"
#include "caster.h"

#include "input.cpp"
#include "caster.cpp"

global bool window_should_close;

int main() {
    QueryPerformanceFrequency((LARGE_INTEGER *)&win32_performance_frequency);
    timeBeginPeriod(1);

    win32_event_arena = make_arena(get_malloc_allocator());

    permanent_arena = arena_alloc(get_virtual_allocator(), MB(2));
    temporary_arena = arena_alloc(get_virtual_allocator(), MB(2));

    char *class_name = "CASTER_WINDOW_CLASS";
    HINSTANCE hinstance = GetModuleHandle(NULL);
    WNDCLASSA window_class{};
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32_proc;
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    window_class.lpszClassName = class_name;
    window_class.hInstance = hinstance;
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    if (!RegisterClassA(&window_class)) {
        printf("RegisterClassA failed, err:%d\n", GetLastError());
    }

    HWND hWnd;
    {
        RECT client_rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW, FALSE);
        hWnd = CreateWindowA(class_name, "Caster", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top, NULL, NULL, hinstance, NULL);
    }

    // ShowCursor(FALSE);

    OS_Handle window_handle = (OS_Handle)hWnd;

    d3d11_render_initialize(hWnd);

    V2_F32 old_window_dim = V2_Zero;

    int target_frames_per_second = 75;
    int target_ms_per_frame = (int)(1000.f / (f32)target_frames_per_second);
    f32 dt = 0.0f;
    s64 start_clock, last_clock;
    start_clock = last_clock = get_wall_clock(); 

    while (!window_should_close) {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                window_should_close = true;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        V2_F32 window_dim = os_get_window_dim(window_handle);
        if (window_dim != old_window_dim) {
            //@Note Do not resize if window is minimized
            if (window_dim.x != 0 && window_dim.y != 0) {
                d3d11_resize_render_target_view((UINT)window_dim.x, (UINT)window_dim.y);
                old_window_dim = window_dim;
            }
        }

        Rect draw_region = make_rect(0.f, 0.f, window_dim.x, window_dim.y);
        r_d3d11_state->draw_region = draw_region;
        
        update_and_render(&win32_events, window_handle, dt / 1000.f);

        win32_events.first = nullptr;
        win32_events.last = nullptr;
        win32_events.count = 0;
        arena_clear(win32_event_arena);

        s64 work_clock = get_wall_clock();
        f32 work_ms_elapsed = get_ms_elapsed(last_clock, work_clock);
        if ((int)work_ms_elapsed < target_ms_per_frame) {
            DWORD sleep_ms = target_ms_per_frame - (int)work_ms_elapsed;
            Sleep(sleep_ms);
        }

        s64 end_clock = get_wall_clock();
        dt = get_ms_elapsed(last_clock, end_clock);

        last_clock = end_clock;
    }

    return 0;
}
