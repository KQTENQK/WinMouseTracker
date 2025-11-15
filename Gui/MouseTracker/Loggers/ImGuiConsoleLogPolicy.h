#ifndef __MOUSE_TRACKER_IMGUI_IMGUICONSOLELOGPOLICY__
#define __MOUSE_TRACKER_IMGUI_IMGUICONSOLELOGPOLICY__

#include "ILogPolicy.h"
#include <vector>
#include <mutex>
#include <chrono>
#include "imgui.h"
#include "LogEntry.h"

namespace Mt
{
    class ImGuiConsoleLogPolicy : public ILogPolicy
    {
        private:
            std::vector<LogEntry> m_logs;
            std::mutex m_mutex;
            bool m_autoScroll;
            size_t m_maxLogs;
            uint64_t m_nextId;
            bool m_showInfo;
            bool m_showWarnings;
            bool m_showErrors;
            bool m_showDebug;

            std::string GetCurrentTimestamp()
            {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
                
                char buffer[64];
                std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&time_t));

                return std::string(buffer) + "." + std::to_string(ms.count());
            }

            void DrawLogEntry(const LogEntry& entry)
            {
                ImVec4 color = LogLevelToColor(entry.Level);
                ImGui::PushStyleColor(ImGuiCol_Text, color);

                ImGui::Text("[%s] [%s] %s", 
                    entry.Timestamp.c_str(), 
                    LogLevelToString(entry.Level), 
                    entry.Message.c_str());

                ImGui::PopStyleColor();
            }

        public:
            ImGuiConsoleLogPolicy(size_t maxLogs = 1000)
            {
                m_autoScroll = true;
                m_maxLogs = maxLogs;
                m_nextId = 0;
                m_showInfo = true;
                m_showWarnings = true;
                m_showErrors = true;
                m_showDebug = true;
            }

            void Write(LogLevel level, const std::string& message) override
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                
                if ((level == LogLevel::Info && !m_showInfo) ||
                    (level == LogLevel::Warning && !m_showWarnings) ||
                    (level == LogLevel::Error && !m_showErrors) ||
                    (level == LogLevel::Debug && !m_showDebug))
                {
                    return;
                }

                m_logs.emplace_back(level, message, GetCurrentTimestamp(), m_nextId++);
                
                if (m_logs.size() > m_maxLogs)
                    m_logs.erase(m_logs.begin());
            }

            void Flush() override {  }

            void Draw(const char* title = "Console", bool* p_open = nullptr)
            {
                ImGui::Begin(title, p_open);

                DrawControlPanel();

                ImGui::Separator();
                
                DrawLogContents();

                ImGui::End();
            }

            void Clear()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_logs.clear();
            }

            size_t GetLogCount() 
            { 
                std::lock_guard<std::mutex> lock(m_mutex);

                return m_logs.size(); 
            }

            const char* GetName() const override
            {
                return "ImGuiConsoleLogPolicy";
            }

            void CopyAllToClipboard()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                std::string clipboardText;
                
                for (const auto& log : m_logs)
                {
                    clipboardText += "[" + log.Timestamp + "] [" + 
                                LogLevelToString(log.Level) + "] " + 
                                log.Message + "\n";
                }
                
                ImGui::SetClipboardText(clipboardText.c_str());
            }

        private:
            void DrawControlPanel()
            {
                if (ImGui::Button("Clear"))
                    Clear();
                    
                ImGui::SameLine();
                ImGui::Checkbox("Auto-scroll", &m_autoScroll);
                
                ImGui::SameLine();
                if (ImGui::Button("Copy All"))
                    CopyAllToClipboard();

                ImGui::SameLine();
                ImGui::Text("Filters:");
                ImGui::SameLine();
                ImGui::Checkbox("Info", &m_showInfo); ImGui::SameLine();
                ImGui::Checkbox("Warn", &m_showWarnings); ImGui::SameLine();
                ImGui::Checkbox("Error", &m_showErrors); ImGui::SameLine();
                ImGui::Checkbox("Debug", &m_showDebug);
            }

            void DrawLogContents()
            {
                ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, 
                                ImGuiWindowFlags_HorizontalScrollbar);

                std::lock_guard<std::mutex> lock(m_mutex);
                
                for (const auto& log : m_logs)
                    DrawLogEntry(log);
                
                if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
                    
                ImGui::EndChild();
            }
    };
}

#endif
