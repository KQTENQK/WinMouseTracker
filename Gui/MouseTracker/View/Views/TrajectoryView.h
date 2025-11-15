#ifndef __MOUSE_TRACKER_IMGUI_TRAJECTORYVIEW__
#define __MOUSE_TRACKER_IMGUI_TRAJECTORYVIEW__

#include "View/IView.h"
#include <vector>
#include <windows.h>
#include "imgui.h"

namespace Mt
{
    class TrajectoryView : public IView
    {
        private:
            std::vector<POINT> m_trajectoryPoints;
            std::string m_displayName;
            bool m_showTable;
            bool m_showGraph;
            ImVec2 m_graphSize;
            float m_pointRadius;
            ImVec4 m_lineColor;
            ImVec4 m_pointColor;
            int m_screenWidth;
            int m_screenHeight;

        public:
            TrajectoryView() 
            {
                m_displayName = "Trajectory";
                m_showTable = true;
                m_showGraph = true;
                m_graphSize = ImVec2(600, 400);
                m_pointRadius = 2.0f;
                m_lineColor = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
                m_pointColor = ImVec4(1.0f, 0.0f, 0.0f, 0.7f);
                m_screenWidth = 1920;
                m_screenHeight = 1080;
            }

            void SetTrajectory(const std::vector<POINT>& points)
            {
                m_trajectoryPoints = points;
            }

            void ClearTrajectory()
            {
                m_trajectoryPoints.clear();
            }

            const std::vector<POINT>& GetTrajectory() const
            {
                return m_trajectoryPoints;
            }

            void Draw() override
            {
                if (!Visible)
                    return;

                ImGui::Begin(GetDisplayName().c_str(), &Visible);

                DrawControls();
                ImGui::Separator();

                if (m_showTable && m_showGraph)
                {
                    float tableWidth = ImGui::GetContentRegionAvail().x * 0.4f;
                    
                    ImGui::BeginChild("TableRegion", ImVec2(tableWidth, 0), true);
                    DrawPointTable();
                    ImGui::EndChild();
                    
                    ImGui::SameLine();
                    
                    ImGui::BeginChild("GraphRegion", ImVec2(0, 0), true);
                    DrawTrajectoryGraph();
                    ImGui::EndChild();
                }

                else if (m_showTable)
                {
                    DrawPointTable();
                }

                else if (m_showGraph)
                {
                    DrawTrajectoryGraph();
                }

                ImGui::End();
            }

            const std::string& GetType() const override 
            { 
                static std::string type = "TrajectoryView";

                return type; 
            }
            
            const std::string& GetCategory() const override 
            { 
                static std::string category = "Views";

                return category; 
            }
            
            const std::string& GetDisplayName() const override 
            { 
                return m_displayName; 
            }

        private:
            void DrawControls()
            {
                ImGui::Text("Points: %zu", m_trajectoryPoints.size());
                ImGui::SameLine();
                
                if (ImGui::Button("Clear"))
                    ClearTrajectory();
                    
                ImGui::SameLine();
                ImGui::Checkbox("Show Table", &m_showTable);
                ImGui::SameLine();
                ImGui::Checkbox("Show Graph", &m_showGraph);

                if (m_showGraph)
                {
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(100);
                    ImGui::SliderFloat("Point Radius", &m_pointRadius, 1.0f, 10.0f);

                    if (ImGui::BeginCombo("Resolution", std::string(std::to_string(m_screenWidth) + "x" + std::to_string(m_screenHeight)).c_str()))
                    {
                        int resolution[2] = { m_screenWidth, m_screenHeight };

                        if (ImGui::InputInt2("Screen Width x Height", resolution))
                        {
                            m_screenWidth = (std::max)(1, resolution[0]);
                            m_screenHeight = (std::max)(1, resolution[1]);
                        }

                        ImGui::EndCombo();
                    }
                }
            }

            void DrawPointTable()
            {
                if (ImGui::BeginTable("TrajectoryPoints", 3, 
                    ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | 
                    ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
                {
                    ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableHeadersRow();

                    int displayStart = 0;
                    int displayEnd = static_cast<int>(m_trajectoryPoints.size());
                    
                    ImGuiListClipper clipper;
                    clipper.Begin(displayEnd - displayStart);
                    
                    while (clipper.Step())
                    {
                        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                        {
                            int index = displayStart + row;
                            
                            if (index >= displayEnd)
                                break;

                            const POINT& point = m_trajectoryPoints[index];
                            
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", index);
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", point.x);
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", point.y);
                        }
                    }
                    
                    ImGui::EndTable();
                }
            }

            void DrawTrajectoryGraph()
            {
                if (m_trajectoryPoints.empty())
                {
                    ImGui::Text("No trajectory data available");

                    return;
                }

                ImVec2 canvasSize = ImGui::GetContentRegionAvail();

                if (canvasSize.x < 50.0f)
                    canvasSize.x = 50.0f;

                if (canvasSize.y < 50.0f)
                    canvasSize.y = 50.0f;

                ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
                
                POINT minPoint = {0, 0};
                POINT maxPoint = {m_screenWidth, m_screenHeight};

                int padding = 50;

                minPoint.x -= padding;
                minPoint.y -= padding;
                maxPoint.x += padding;
                maxPoint.y += padding;
                
                for (const auto& point : m_trajectoryPoints)
                {
                    minPoint.x = (std::min)(minPoint.x, point.x);
                    minPoint.y = (std::min)(minPoint.y, point.y);
                    maxPoint.x = (std::max)(maxPoint.x, point.x);
                    maxPoint.y = (std::max)(maxPoint.y, point.y);
                }

                float width = static_cast<float>(maxPoint.x - minPoint.x);
                float height = static_cast<float>(maxPoint.y - minPoint.y);
                
                if (width <= 0)
                    width = 1.0f;

                if (height <= 0)
                    height = 1.0f;

                ImDrawList* drawList = ImGui::GetWindowDrawList();

                drawList->AddRect
                (
                    canvasPosition, 
                    ImVec2(canvasPosition.x + canvasSize.x, canvasPosition.y + canvasSize.y), 
                    IM_COL32(255, 255, 255, 255)
                );

                for (size_t i = 1; i < m_trajectoryPoints.size(); i++)
                {
                    const POINT& previous = m_trajectoryPoints[i - 1];
                    const POINT& current = m_trajectoryPoints[i];

                    ImVec2 previousPosition = WorldToScreen(previous, minPoint, width, height, canvasPosition, canvasSize);
                    ImVec2 currentPosition = WorldToScreen(current, minPoint, width, height, canvasPosition, canvasSize);

                    drawList->AddLine
                    (
                        previousPosition,
                        currentPosition, 
                        ImColor(m_lineColor),
                        2.0f
                    );

                    if (m_pointRadius > 0)
                    {
                        drawList->AddCircleFilled
                        (
                            currentPosition,
                            m_pointRadius,
                            ImColor(m_pointColor)
                        );
                    }
                }

                if (!m_trajectoryPoints.empty())
                {
                    ImVec2 startPosition = WorldToScreen(m_trajectoryPoints.front(), minPoint, width, height, canvasPosition, canvasSize);
                    drawList->AddCircleFilled(startPosition, m_pointRadius * 1.5f, IM_COL32(0, 255, 0, 255));

                    ImVec2 endPosition = WorldToScreen(m_trajectoryPoints.back(), minPoint, width, height, canvasPosition, canvasSize);
                    drawList->AddCircleFilled(endPosition, m_pointRadius * 1.5f, IM_COL32(255, 0, 0, 255));
                }

                ImGui::Dummy(canvasSize);
            }

            ImVec2 WorldToScreen
            (
                const POINT& worldPoint,
                const POINT& minPoint, 
                float width,
                float height,
                const ImVec2& canvasPosition,
                const ImVec2& canvasSize
            )
            {
                float x = ((worldPoint.x - minPoint.x) / width) * canvasSize.x;
                float y = ((worldPoint.y - minPoint.y) / height) * canvasSize.y;
                
                return ImVec2(canvasPosition.x + x, canvasPosition.y + y);
            }
    };
}

#endif
