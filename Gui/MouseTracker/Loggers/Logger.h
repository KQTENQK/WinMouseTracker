#ifndef __MOUSE_TRACKER_IMGUI_LOGGER__
#define __MOUSE_TRACKER_IMGUI_LOGGER__

#include "ILogPolicy.h"
#include "ImGuiConsoleLogPolicy.h"
#include "LogLevel.h"
#include <vector>
#include <memory>
#include <mutex>

namespace Mt
{
    class Logger
    {
        private:
            std::vector<std::unique_ptr<ILogPolicy>> m_policies;
            std::mutex m_mutex;
            LogLevel m_minLevel;

            Logger()
            {
                m_minLevel = LogLevel::Info;
            }
            
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

        public:
            static Logger& GetInstance()
            {
                static Logger instance;

                return instance;
            }

            template<typename T, typename... Args>
            Logger& WithPolicy(Args&&... args)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_policies.push_back(std::make_unique<T>(std::forward<Args>(args)...));

                return *this;
            }

            Logger& AddPolicy(std::unique_ptr<ILogPolicy> policy)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_policies.push_back(std::move(policy));
                return *this;
            }

            Logger& SetMinLevel(LogLevel level)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_minLevel = level;

                return *this;
            }

            void Log(LogLevel level, const std::string& message)
            {
                if (static_cast<int>(level) < static_cast<int>(m_minLevel))
                    return;

                std::lock_guard<std::mutex> lock(m_mutex);

                for (auto& policy : m_policies)
                    policy->Write(level, message);
            }

            void Info(const std::string& message)
            {
                Log(LogLevel::Info, message);
            }

            void Warning(const std::string& message)
            {
                Log(LogLevel::Warning, message);
            }

            void Error(const std::string& message)
            {
                Log(LogLevel::Error, message);
            }

            void Debug(const std::string& message)
            {
                Log(LogLevel::Debug, message);
            }

            template<typename... Args>
            void InfoF(const char* format, Args&&... args)
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
                Info(buffer);
            }

            template<typename... Args>
            void WarningF(const char* format, Args&&... args)
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
                Warning(buffer);
            }

            template<typename... Args>
            void ErrorF(const char* format, Args&&... args)
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
                Error(buffer);
            }

            template<typename... Args>
            void DebugF(const char* format, Args&&... args)
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
                Debug(buffer);
            }

            void ClearPolicies()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_policies.clear();
            }

            size_t GetPolicyCount()
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                return m_policies.size();
            }

            void Flush()
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                for (auto& policy : m_policies)
                    policy->Flush();
            }

            ImGuiConsoleLogPolicy* GetImGuiConsole()
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                for (auto& policy : m_policies)
                    if (auto* console = dynamic_cast<ImGuiConsoleLogPolicy*>(policy.get()))
                        return console;

                return nullptr;
            }
    };
}

#endif
