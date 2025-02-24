#ifndef INPUT_H
#define INPUT_H

enum Button_State {
    ButtonState_Down      = (1<<0),
    ButtonState_Pressed   = (1<<1),
    ButtonState_Released  = (1<<3),
};
EnumDefineFlagOperators(Button_State);

struct Input {
    Button_State buttons[OS_KEY_COUNT];

    V2_S32 mouse_position;
    V2_S32 last_mouse_position;

    V2_S32 mouse_drag_start;
    b32 mouse_dragging;

    V2_S32 scroll_delta;

    V2_S32 client_dim;

    b32 capture_cursor;
};

internal void input_begin(OS_Handle window_handle, OS_Event *events);
internal void input_end(OS_Handle window_handle);

#endif // INPUT_H
