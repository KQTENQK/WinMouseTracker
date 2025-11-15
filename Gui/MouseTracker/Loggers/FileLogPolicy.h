#ifndef __MOUSE_TRACKER_IMGUI_FILELOGPOLICY__
#define __MOUSE_TRACKER_IMGUI_FILELOGPOLICY__

#include "ILogPolicy.h"
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Mt
{
    class FileLogPolicy : public ILogPolicy
    {
        private:
            std::ofstream m_file;
            std::mutex m_mutex;
            std::string m_filename;

            std::string GetCurrentTimestamp()
            {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);

                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;
                
                std::stringstream ss;

                ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
                ss << "." << std::setfill('0') << std::setw(3) << ms.count();

                return ss.str();
            }

        public:
            FileLogPolicy(const std::string& filename)
            {
                m_filename = filename;
                
                m_file.open(filename, std::ios::app);

                if (m_file.is_open())
                {
                    m_file << "\n=== Logging started at " << GetCurrentTimestamp() << " ===\n";
                    m_file.flush();
                }
            }

            ~FileLogPolicy()
            {
                if (m_file.is_open())
                {
                    m_file << "=== Logging ended at " << GetCurrentTimestamp() << " ===\n\n";
                    m_file.close();
                }
            }

            void Write(LogLevel level, const std::string& message) override
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                
                if (m_file.is_open())
                {
                    m_file << "[" << GetCurrentTimestamp() << "] "
                        << "[" << LogLevelToString(level) << "] "
                        << message << std::endl;
                }
            }

            void Flush() override
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                if (m_file.is_open())
                    m_file.flush();
            }

            const char* GetName() const override
            {
                return "FileLogPolicy";
            }
    };
}

#endif
