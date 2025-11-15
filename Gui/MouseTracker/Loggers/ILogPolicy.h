#ifndef __MOUSE_TRACKER_IMGUI_ILOGPOLICY__
#define __MOUSE_TRACKER_IMGUI_ILOGPOLICY__

#include "LogLevel.h"
#include <string>

namespace Mt
{
    class ILogPolicy
    {
        public:
            virtual ~ILogPolicy() = default;
            virtual void Write(LogLevel level, const std::string& message) = 0;
            virtual void Flush() = 0;
            virtual const char* GetName() const = 0;
    };
}

#endif
