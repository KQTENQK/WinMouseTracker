#ifndef __MOUSE_TRACKER_IMGUI_HOTKEYREGISTRATION__
#define __MOUSE_TRACKER_IMGUI_HOTKEYREGISTRATION__

#include <windows.h>
#include <functional>

namespace Mt
{
    struct HotkeyRegistration
    {
        int Id;
        UINT Modifiers;
        UINT Key;
        std::function<void()> Callback;
        bool Enabled;
    };
}

#endif
