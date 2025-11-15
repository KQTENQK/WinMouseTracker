#ifndef __MOUSE_TRACKER_IMGUI_MOUSETRACKER__
#define __MOUSE_TRACKER_IMGUI_MOUSETRACKER__

#include "MouseTracker.h"
#include "View/ViewRegistry.h"
#include "View/Views/ViewMenu.h"
#include "View/Views/ConsoleView.h"
#include "View/Views/TrajectoryView.h"
#include "View/Views/MouseTrackerView.h"
#include "Loggers/Logger.h"
#include "Loggers/ImGuiConsoleLogPolicy.h"
#include "Loggers/FileLogPolicy.h"
#include "Hotkeys/WinApiHotkeyManager.h"
#include "FileOperations/WinApiFileOperations.h"
#include "FileOperations/TrajectoryFileOperations.h"
#include <memory>

namespace Mt
{
    class MouseTracker
    {
        private:
            std::unique_ptr<ViewMenu> m_viewMenu;

        public:
            MouseTracker() = default;

            void Initialize()
            {
                Logger::GetInstance()
                    .WithPolicy<ImGuiConsoleLogPolicy>(1000)
                    //.WithPolicy<FileLogPolicy>("mouse_tracker.log")
                    .SetMinLevel(LogLevel::Info);

                auto& hotkeyManager = WinApiHotkeyManager::GetInstance();
                auto& registry = ViewRegistry::GetInstance();

                registry.RegisterView<ConsoleView>("Console", "Views", "Console");
                registry.RegisterView<TrajectoryView>("TrajectoryView", "Views", "Trajectory");
                registry.RegisterView<MouseTrackerView>("MouseTrackerView", "Views", "Mouse Tracker");

                m_viewMenu = std::make_unique<ViewMenu>(&registry.GetInstance());

                m_viewMenu->SetFileCallbacks
                (
                    [this]() { TrajectoryFileOperations::SaveTrajectoryWindowsCtxAsync(); },
                    [this]() { TrajectoryFileOperations::LoadTrajectoryWindowsCtxAsync(); }
                );

                registry.InitializeAllViews();

                auto* trajectoryView = dynamic_cast<TrajectoryView*>(registry.GetView("TrajectoryView"));
                auto* mouseTrackerView = dynamic_cast<MouseTrackerView*>(registry.GetView("MouseTrackerView"));
                
                if (trajectoryView && mouseTrackerView)
                    mouseTrackerView->SetTrajectoryView(trajectoryView);

                Logger::GetInstance().Debug("Mouse Tracker initialized");
            }

            void Show()
            {
                HandleImGuiHotkeys();

                if (ImGui::BeginMainMenuBar())
                {
                    m_viewMenu->Draw();
                    ImGui::EndMainMenuBar();
                }

                auto& registry = ViewRegistry::GetInstance();

                for (const auto& viewType : registry.GetViewOrder())
                    if (auto* view = registry.GetView(viewType))
                        if (view->IsVisible())
                            view->Draw();
            }

            void Shutdown()
            {
                WinApiHotkeyManager::GetInstance().StopListening();
                Logger::GetInstance().Info("Mouse Tracker shutting down");
            }
            
            static MouseTracker& GetInstance()
            {
                static MouseTracker instance;

                return instance;
            }

        private:
            void HandleImGuiHotkeys()
            {
                ImGuiIO& io = ImGui::GetIO();
        
                if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
                    if (m_viewMenu)
                        m_viewMenu->SaveTrajectory();
                
                if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O))
                    if (m_viewMenu)
                        m_viewMenu->LoadTrajectory();
            }
    };
}

#endif
