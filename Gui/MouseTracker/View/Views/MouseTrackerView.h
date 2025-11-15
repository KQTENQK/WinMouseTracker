#ifndef __MOUSE_TRACKER_IMGUI_MOUSETRACKERVIEW__
#define __MOUSE_TRACKER_IMGUI_MOUSETRACKERVIEW__

#include "View/IView.h"
#include "TrajectoryRecorder.h"
#include "TrajectoryView.h"
#include "FileOperations/WinApiFileOperations.h"
#include "FileOperations/TrajectoryFileOperations.h"
#include "Hotkeys/WinApiHotkeyManager.h"
#include "imgui.h"
#include <thread>
#include <atomic>
#include <functional>
#include <fstream>
#include <map>
#include <filesystem>
#include <future>

namespace Mt
{
    class MouseTrackerView : public IView
    {
        private:
            TrajectoryRecorder m_recorder;
            TrajectoryView* m_trajectoryView;
            
            int m_delay;
            int m_delta;
            int m_endDelay;
            int m_count;
            enum class RecordingMode { Standard, Continuous } m_recordingMode;
            
            std::string m_outputDirectory;
            std::string m_baseFilename;
            int m_fileCounter;
            
            std::atomic<bool> m_isRecording;
            std::atomic<bool> m_shouldStop;
            std::thread m_recordingThread;
            std::atomic<bool> m_threadShouldExit;
            std::condition_variable m_recordingCV;
            std::mutex m_recordingMutex;
            std::atomic<bool> m_recordingRequested;

            std::map<std::string, std::pair<UINT, UINT>> m_hotkeyPresets;
            std::string m_currentHotkey;
            bool m_hotkeysEnabled;
            
            std::function<void()> m_onRecordingStart;
            std::function<void()> m_onRecordingStop;

        public:
            MouseTrackerView()
            {
                m_delay = 1000;
                m_delta = 1;
                m_endDelay = 2000;
                m_recordingMode = RecordingMode::Continuous;
                m_outputDirectory = ".";
                m_baseFilename = "trajectory";
                m_fileCounter = 1;
                m_isRecording = false;
                m_trajectoryView = nullptr;
                m_hotkeysEnabled = false;
                m_count = 1;
                m_threadShouldExit = false;
                m_recordingRequested = false;

                InitializeHotkeyPresets();
                m_currentHotkey = "Ctrl + R";

                UpdateNextFileCounter();
                RegisterHotkeys();

                m_recordingThread = std::thread([this]() { this->RecordingWorker(); });

                WinApiHotkeyManager::GetInstance().StartListening();
            }

            void SetTrajectoryView(TrajectoryView* trajectoryView)
            {
                m_trajectoryView = trajectoryView;
            }

            void SetCallbacks(std::function<void()> onStart, std::function<void()> onStop)
            {
                m_onRecordingStart = onStart;
                m_onRecordingStop = onStop;
            }

            void Draw() override
            {
                if (!Visible)
                    return;

                ImGui::Begin(GetDisplayName().c_str(), &Visible);

                DrawModeSelection();
                ImGui::Separator();
                DrawHotkeySettings();
                ImGui::Separator();
                DrawParameters();
                ImGui::Separator();
                DrawFileSettings();
                ImGui::Separator();
                DrawControls();

                ImGui::End();
            }

            void StartRecording()
            {
                if (m_isRecording)
                    return;

                m_isRecording = true;
                m_shouldStop = false;
                
                if (m_onRecordingStart)
                    m_onRecordingStart();

                m_recordingRequested = true;
                m_recordingCV.notify_one();
            }

            void StopRecording()
            {
                if (!m_isRecording)
                    return;

                m_shouldStop = true;
                m_isRecording = false;
                
                if (m_onRecordingStop)
                    m_onRecordingStop();
            }

            void ToggleRecording()
            {
                if (m_isRecording)
                    StopRecording();
                else
                    StartRecording();
            }

            bool IsRecording() const
            {
                return m_isRecording;
            }

            const std::string& GetType() const override 
            { 
                static std::string type = "MouseTrackerView";

                return type; 
            }
            
            const std::string& GetCategory() const override 
            { 
                static std::string category = "Views";

                return category; 
            }
            
            const std::string& GetDisplayName() const override 
            { 
                static std::string name = "Mouse Tracker";

                return name; 
            }

            ~MouseTrackerView()
            {
                StopRecording();
                
                m_threadShouldExit = true;
                m_recordingCV.notify_all();
                
                if (m_recordingThread.joinable())
                    m_recordingThread.join();
                
                UnregisterHotkeys();
                WinApiHotkeyManager::GetInstance().StopListening();
            }

        private:
            void RecordingWorker()
            {
                while (!m_threadShouldExit)
                {
                    std::unique_lock<std::mutex> lock(m_recordingMutex);

                    m_recordingCV.wait(lock, [this]()
                    {
                        return m_recordingRequested || m_threadShouldExit;
                    });
                    
                    if (m_threadShouldExit)
                        break;
                        
                    m_recordingRequested = false;
                    lock.unlock();

                    RecordingThread();
                }
            }

            void InitializeHotkeyPresets()
            {
                m_hotkeyPresets =
                {
                    { "F9", {0, VK_F9} },
                    { "F10", {0, VK_F10} },
                    { "F11", {0, VK_F11} },
                    { "F12", {0, VK_F12} },
                    { "Ctrl + R", {MOD_CONTROL, 'R'} },
                    { "Ctrl + Shift + R", {MOD_CONTROL | MOD_SHIFT, 'R'} }
                };
            }

            void RegisterHotkeys()
            {
                auto& hotkeyManager = WinApiHotkeyManager::GetInstance();
                auto hotkey = m_hotkeyPresets[m_currentHotkey];
                
                hotkeyManager.RegisterHotkey
                (
                    hotkey.first,
                    hotkey.second,
                    [this]()
                    {
                        this->ToggleRecording();
                    }
                );

                m_hotkeysEnabled = true;
            }

            void UnregisterHotkeys()
            {
                if (m_hotkeysEnabled)
                {
                    WinApiHotkeyManager::GetInstance().UnregisterAll();
                    m_hotkeysEnabled = false;
                }
            }

            void DrawModeSelection()
            {
                ImGui::Text("Recording Mode:");
                ImGui::SameLine();
                
                if (ImGui::RadioButton("Standard", m_recordingMode == RecordingMode::Standard))
                    m_recordingMode = RecordingMode::Standard;
                
                ImGui::SameLine();
                
                if (ImGui::RadioButton("Continuous", m_recordingMode == RecordingMode::Continuous))
                    m_recordingMode = RecordingMode::Continuous;
            }

            void DrawParameters()
            {
                switch (m_recordingMode)
                {
                    case RecordingMode::Standard:

                        ImGui::SetNextItemWidth(200);

                        if (ImGui::InputInt("Delay (ms)", &m_delay, 100, 1000))
                            m_delay = (std::max)(1, m_delay);
                        
                        ImGui::SameLine();
                        ImGui::TextDisabled("(?)");

                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Stop recording after mouse is stationary for this many milliseconds.");

                        ImGui::SetNextItemWidth(200);

                        if (ImGui::InputInt("Iterations", &m_count, 100, 1000))
                            m_count = (std::max)(1, m_count);
                        
                        ImGui::SameLine();
                        ImGui::TextDisabled("(?)");

                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Amount of recording iterations to be performed.");

                        break;
                    
                    case RecordingMode::Continuous:

                        ImGui::SetNextItemWidth(200);

                        if (ImGui::InputInt("End Delay (ms)", &m_endDelay, 100, 1000))
                            m_endDelay = (std::max)(1, m_endDelay);
                        
                        ImGui::SameLine();
                        ImGui::TextDisabled("(?)");

                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Stop recording when no movement detected for this duration.");

                        break;
                    
                    default:
                        throw std::exception("Undefined recording mode option.");
                }

                ImGui::SetNextItemWidth(200);

                if (ImGui::InputInt("Delta (ms)", &m_delta, 1, 10))
                    m_delta = (std::max)(1, m_delta);
                
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Sampling interval between points.");
            }

            void DrawFileSettings()
            {
                ImGui::Text("Output Settings:");
                
                bool directoryChanged = false;
                ImGui::SetNextItemWidth(300);

                char buffer[256] = "";
                strncpy(buffer, m_outputDirectory.c_str(), IM_ARRAYSIZE(buffer) - 1);
                buffer[IM_ARRAYSIZE(buffer) - 1] = '\0';

                if (ImGui::InputText("Output Directory", buffer, IM_ARRAYSIZE(buffer)))
                    directoryChanged = true;

                m_outputDirectory = std::string(buffer);
                
                ImGui::SameLine();

                if (ImGui::Button("Browse##Dir"))
                {
                    std::string selectedDir = WinApiFileOperations::SelectFolderDialog(m_outputDirectory);

                    if (!selectedDir.empty())
                    {
                        m_outputDirectory = selectedDir;
                        directoryChanged = true;
                    }
                }

                bool filenameChanged = false;

                ImGui::SetNextItemWidth(200);

                strncpy(buffer, m_baseFilename.c_str(), IM_ARRAYSIZE(buffer) - 1);
                buffer[IM_ARRAYSIZE(buffer) - 1] = '\0';

                if (ImGui::InputText("Base Filename", buffer, IM_ARRAYSIZE(buffer)))
                    filenameChanged = true;

                m_baseFilename = std::string(buffer);

                if (directoryChanged || filenameChanged)
                    UpdateNextFileCounter();
                
                ImGui::SameLine();
                ImGui::Text("Next: %s_%d.crsdat", m_baseFilename.c_str(), m_fileCounter);
            }

            void DrawHotkeySettings()
            {
                ImGui::Text("Hotkey Settings:");
                
                ImGui::SetNextItemWidth(120);

                std::string lastHotkey = m_currentHotkey;

                if (ImGui::BeginCombo("Hotkey", m_currentHotkey.c_str()))
                {
                    for (const auto& hotkey : m_hotkeyPresets)
                    {
                        bool isSelected = (m_currentHotkey == hotkey.first);

                        if (ImGui::Selectable(hotkey.first.c_str(), isSelected))
                            m_currentHotkey = hotkey.first;

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }
                
                if (m_currentHotkey != lastHotkey)
                {
                    UnregisterHotkeys();
                    RegisterHotkeys();
                }
            }

            void DrawControls()
            {
                if (!m_isRecording)
                {
                    if (ImGui::Button("Start Recording", ImVec2(120, 30)))
                        StartRecording();
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                    
                    if (ImGui::Button("Stop Recording", ImVec2(120, 30)))
                        StopRecording();
                    
                    ImGui::PopStyleColor(2);
                    
                    ImGui::SameLine();
                }
            }

            void RecordingThread()
            {
                try
                {
                    std::vector<POINT> trajectory;
                    
                    switch (m_recordingMode)
                    {
                        case RecordingMode::Standard:
                            trajectory = m_recorder.StartReadCursorRoutineTscCpuWait(m_count, m_delta);

                            if (!trajectory.empty() && !m_shouldStop)
                            {
                                if (m_trajectoryView)
                                    m_trajectoryView->SetTrajectory(trajectory);

                                std::string filename = m_outputDirectory + "\\" + m_baseFilename + "_" + 
                                    std::to_string(m_fileCounter) + ".crsdat";

                                TrajectoryFileOperations::SaveTrajectoryAsync(trajectory, m_outputDirectory, filename);
                                m_fileCounter++;
                            }

                            StopRecording();

                            break;

                        case RecordingMode::Continuous:
                            while (m_isRecording && !m_shouldStop)
                            {
                                trajectory = m_recorder.StartReadMouseTrajectoryRoutineTscCpuWait(m_endDelay, m_delta);

                                if (!trajectory.empty() && !m_shouldStop)
                                {
                                    if (m_trajectoryView)
                                        m_trajectoryView->SetTrajectory(trajectory);

                                    std::string filename = m_outputDirectory + "\\" + m_baseFilename + "_" + 
                                    std::to_string(m_fileCounter) + ".crsdat";

                                    TrajectoryFileOperations::SaveTrajectoryAsync(trajectory, m_outputDirectory, filename);
                                    m_fileCounter++;
                                }
                            }
                                
                            break;

                        default:
                            throw std::exception("Undefined recording mode.");
                    }

                }
                catch (const std::exception& e)
                {
                    Logger::GetInstance().ErrorF("Recording error: %s", e.what());
                }

                m_isRecording = false;
            }

            void UpdateNextFileCounter()
            {
                if (!WinApiFileOperations::DirectoryExists(m_outputDirectory))
                {
                    m_fileCounter = 1;

                    return;
                }

                m_fileCounter = 1;
                std::string pattern = m_baseFilename + "_*.crsdat";
                
                try
                {
                    for (const auto& entry : std::filesystem::directory_iterator(m_outputDirectory))
                    {
                        if (entry.is_regular_file())
                        {
                            std::string filename = entry.path().filename().string();
                            
                            if (filename.find(m_baseFilename + "_") == 0)
                            {
                                m_fileCounter++;
                            }
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    Logger::GetInstance().WarningF("Error scanning directory: %s", e.what());
                    m_fileCounter = 1;
                }
            }
    };
}

#endif
