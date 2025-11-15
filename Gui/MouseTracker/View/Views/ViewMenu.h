#ifndef __MOUSE_TRACKER_IMGUI_VIEWMENU__
#define __MOUSE_TRACKER_IMGUI_VIEWMENU__

#include "View/ViewRegistry.h"
#include "imgui.h"
#include <functional>

namespace Mt
{
    class ViewMenu
    {
        private:
            ViewRegistry* m_registry;
            
            std::function<void()> m_saveTrajectoryCallback;
            std::function<void()> m_loadTrajectoryCallback;

        public:
            ViewMenu
            (
                ViewRegistry* registry
            )
            {
                m_registry = registry;
            }
            
            void SetFileCallbacks
            (
                std::function<void()> saveTrajectoryCallback,
                std::function<void()> loadTrajectoryCallback
            )
            {
                m_saveTrajectoryCallback = saveTrajectoryCallback;
                m_loadTrajectoryCallback = loadTrajectoryCallback;
            }

            void SaveTrajectory()
            {
                m_saveTrajectoryCallback();
            }

            void LoadTrajectory()
            {
                m_loadTrajectoryCallback();
            }
            
            void Draw()
            {
                DrawFileMenu();
                DrawViewMenu();
            }
            
        private:
            void DrawFileMenu()
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save Trajectory", "Ctrl + S"))
                        if (m_saveTrajectoryCallback)
                                m_saveTrajectoryCallback();

                    ImGui::Separator();
                    
                    if (ImGui::MenuItem("Load Trajectory", "Ctrl + O"))
                        if (m_loadTrajectoryCallback)
                                m_loadTrajectoryCallback();

                    ImGui::EndMenu();
                }
            }
            
            void DrawViewMenu()
            {
                if (ImGui::BeginMenu("View"))
                {
                    DrawViewCategory("Views");

                    ImGui::EndMenu();
                }
            }
            
            void DrawViewCategory(const std::string& category)
            {
                auto viewTypes = m_registry->GetViewTypesByCategory(category);

                if (viewTypes.empty())
                    return;
                
                if (ImGui::BeginMenu(category.c_str()))
                {
                    for (const auto& type : viewTypes)
                    {
                        if (auto* view = m_registry->GetView(type))
                        {
                            bool visible = view->IsVisible();

                            if (ImGui::MenuItem(view->GetDisplayName().c_str(), nullptr, &visible))
                                view->SetVisible(visible);
                        }
                    }
                    
                    ImGui::EndMenu();
                }
            }
    };
}

#endif
