global Input g_input;

#define button_down(Button)     (g_input.buttons[(Button)] & ButtonState_Down)
#define button_pressed(Button)  (g_input.buttons[(Button)] & ButtonState_Pressed)
#define button_released(Button) (g_input.buttons[(Button)] & ButtonState_Released)

internal Button_State get_button_state(OS_Key key) { return g_input.buttons[key]; }
internal bool key_up(OS_Key key)      { bool result = button_released(key); return result; }
internal bool key_pressed(OS_Key key) { bool result = button_pressed(key); return result; }
internal bool key_down(OS_Key key)    { bool result = button_down(key); return result; }

internal inline V2_F32 get_mouse_drag_delta() {
    V2_F32 result = {};
    result.x = (f32)(g_input.mouse_position.x - g_input.mouse_drag_start.x);
    result.y = (f32)(g_input.mouse_position.y - g_input.mouse_drag_start.y);
    return result;
}

internal inline V2_F32 get_mouse_delta() {
    V2_F32 result = {};
    if (g_input.capture_cursor) {
        result.x = (f32)(g_input.mouse_position.x - g_input.last_mouse_position.x);
        result.y = (f32)(g_input.mouse_position.y - g_input.last_mouse_position.y);
    }
    return result;
}

internal inline V2_F32 mouse_position() {
    V2_F32 result;
    result.x = (f32)g_input.mouse_position.x;
    result.y = (f32)(g_input.client_dim.y - g_input.mouse_position.y);
    return result;
}

internal void input_begin(OS_Handle window_handle, OS_Event_List *events) {
    V2_F32 window_dim = os_get_window_dim(window_handle);
    g_input.client_dim = v2_s32((s32)window_dim.x, (s32)window_dim.y);

    for (OS_Event *evt = events->first; evt; evt = evt->next) {
        bool pressed = false;
        switch (evt->kind) {
        case OS_EventKind_MouseMove:
            g_input.mouse_position = evt->pos;
            break;

        case OS_EventKind_Scroll:
            g_input.scroll_delta = evt->delta;
            break;

        case OS_EventKind_MouseDown:
            g_input.mouse_dragging = true;
            g_input.mouse_drag_start = evt->pos;
            g_input.mouse_position = evt->pos;
        case OS_EventKind_Press:
            pressed = true;
        case OS_EventKind_Release:
        case OS_EventKind_MouseUp:
            if (evt->kind == OS_EventKind_MouseUp) g_input.mouse_dragging = false;

            {
                Button_State state = {};
                state |= (Button_State)(ButtonState_Pressed*(pressed && !button_down(evt->key)));
                state |= (Button_State)(ButtonState_Down*pressed);
                state |= (Button_State)(ButtonState_Released * (!pressed));
                g_input.buttons[evt->key] = state;
                break;
            }
        }
    }
}

internal void input_end(OS_Handle window_handle) {
    V2_F32 window_dim = os_get_window_dim(window_handle);
    V2_S32 client_dim = v2_s32((s32)window_dim.x, (s32)window_dim.y);
    g_input.scroll_delta = v2_s32(0, 0);
    g_input.mouse_drag_start = g_input.mouse_position;
    for (int i = 0; i < ArrayCount(g_input.buttons); i++) {
        g_input.buttons[i] &= ~ButtonState_Released;
        g_input.buttons[i] &= ~ButtonState_Pressed;
    }

    if (os_window_is_focused(window_handle) && g_input.capture_cursor) {
        os_set_cursor(OS_Cursor_Hidden);
        V2_S32 center = v2_s32(client_dim.x / 2, client_dim.y / 2);
        g_input.mouse_position.x = g_input.last_mouse_position.x = center.x;
        g_input.mouse_position.y = g_input.last_mouse_position.y = center.y;

        POINT pt = {center.x, center.y};
        ClientToScreen((HWND)window_handle, &pt);
        SetCursorPos(pt.x, pt.y);
        // https://github.com/libsdl-org/SDL/blob/38e3c6a4aa338d062ca2eba80728bfdf319f7104/src/video/windows/SDL_windowsmouse.c#L319
        // SetCursorPos(pt.x + 1, pt.y);
        // SetCursorPos(pt.x, pt.y);
    } else {
        g_input.last_mouse_position.x = g_input.mouse_position.x;
        g_input.last_mouse_position.y = g_input.mouse_position.y;
    }
}
