#ifndef __MOUSE_TRACKER_IMGUI_LOGLEVEL__
#define __MOUSE_TRACKER_IMGUI_LOGLEVEL__

#include "imgui.h"

namespace Mt
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error,
        Debug
    };

    inline const char* LogLevelToString(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Debug: return "DEBUG";
            default: return "UNKNOWN";
        }
    }

    inline ImVec4 LogLevelToColor(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::Info: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            case LogLevel::Warning: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
            case LogLevel::Error: return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            case LogLevel::Debug: return ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
            default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

#endif
