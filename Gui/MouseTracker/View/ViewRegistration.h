#ifndef __MOUSE_TRACKER_IMGUI_VIEWREGISTRATION__
#define __MOUSE_TRACKER_IMGUI_VIEWREGISTRATION__

#include <string>
#include <functional>
#include <memory>
#include "IView.h"

namespace Mt
{
    struct ViewRegistration
    {
        std::string Type;
        std::string Category;
        std::string DisplayName;
        std::function<std::unique_ptr<IView>()> Instantiate;
        bool EnabledByDefault = true;
    };
}

#endif
