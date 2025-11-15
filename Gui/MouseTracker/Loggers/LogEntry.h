#ifndef __MOUSE_TRACKER_IMGUI_LOGENTRY__
#define __MOUSE_TRACKER_IMGUI_LOGENTRY__

#include "LogLevel.h"
#include <string>

namespace Mt
{
    struct LogEntry
    {
        LogLevel Level;
        std::string Message;
        std::string Timestamp;
        uint64_t Id;

        LogEntry
        (
            LogLevel level,
            const std::string& message,
            const std::string& timestamp,
            uint64_t id
        )
        {
            Level = level;
            Message = message;
            Timestamp = timestamp;
            Id = id;
        }
    };
}

#endif
