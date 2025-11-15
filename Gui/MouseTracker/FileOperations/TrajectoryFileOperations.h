#ifndef __MOUSE_TRACKER_IMGUI_TRAJECTORYFILEOPERATIONS__
#define __MOUSE_TRACKER_IMGUI_TRAJECTORYFILEOPERATIONS__

#include "View/Views/TrajectoryView.h"
#include "FileOperations/WinApiFileOperations.h"
#include "View/ViewRegistry.h"
#include "Loggers/Logger.h"
#include <fstream>

namespace Mt
{
    class TrajectoryFileOperations
    {
        public:
            static void SaveTrajectoryWindowsCtx()
            {
                auto* trajectoryView = dynamic_cast<TrajectoryView*>
                (
                    ViewRegistry::GetInstance().GetView("TrajectoryView")
                );
                
                if (!trajectoryView)
                {
                    Logger::GetInstance().Error("Trajectory view not found");

                    return;
                }

                const auto& trajectory = trajectoryView->GetTrajectory();

                if (trajectory.empty())
                {
                    Logger::GetInstance().Warning("No trajectory data to save");

                    return;
                }

                std::string filename = WinApiFileOperations::SaveFileDialog
                (
                    "",
                    {{ "Trajectory Files (.crsdat)", "*.crsdat" }, { "All Files", "*.*" }}
                );

                if (filename.empty())
                    return;

                std::ofstream file(filename);

                if (file.is_open())
                {
                    for (const auto& point : trajectory)
                        file << point.x << ";" << point.y << "\n";

                    file.close();

                    Logger::GetInstance().InfoF("Trajectory saved to: %s", filename.c_str());
                }
                else
                {
                    Logger::GetInstance().ErrorF("Failed to save trajectory to: %s", filename.c_str());
                }
            }

            static void LoadTrajectoryWindowsCtx()
            {
                auto* trajectoryView = dynamic_cast<TrajectoryView*>
                (
                    ViewRegistry::GetInstance().GetView("TrajectoryView")
                );
                
                if (!trajectoryView)
                {
                    Logger::GetInstance().Error("Trajectory view not found");

                    return;
                }

                std::string filename = WinApiFileOperations::OpenFileDialog
                (
                    "",
                    {{ "Trajectory Files (.crsdat)", "*.crsdat" }, { "All Files", "*.*" }}
                );

                if (filename.empty())
                    return;

                std::vector<POINT> trajectory;
                std::ifstream file(filename);
                std::string line;

                if (file.is_open())
                {
                    while (std::getline(file, line))
                    {
                        size_t delimiter = line.find(';');

                        if (delimiter != std::string::npos)
                        {
                            try
                            {
                                POINT point;
                                point.x = std::stoi(line.substr(0, delimiter));
                                point.y = std::stoi(line.substr(delimiter + 1));
                                trajectory.push_back(point);
                            }
                            catch (const std::exception& e)
                            {
                                Logger::GetInstance().WarningF("Invalid line in trajectory file: %s", line.c_str());
                            }
                        }
                    }

                    file.close();

                    trajectoryView->SetTrajectory(trajectory);

                    Logger::GetInstance().InfoF("Trajectory loaded from: %s (%zu points)", 
                                            filename.c_str(), trajectory.size());
                }
                else
                {
                    Logger::GetInstance().ErrorF("Failed to load trajectory from: %s", filename.c_str());
                }
            }

            static void SaveTrajectoryWindowsCtxAsync()
            {
                auto* trajectoryView = dynamic_cast<TrajectoryView*>
                (
                    ViewRegistry::GetInstance().GetView("TrajectoryView")
                );
                
                if (!trajectoryView)
                {
                    Logger::GetInstance().Error("Trajectory view not found");

                    return;
                }

                const auto& trajectory = trajectoryView->GetTrajectory();

                if (trajectory.empty())
                {
                    Logger::GetInstance().Warning("No trajectory data to save");

                    return;
                }

                std::string filename = WinApiFileOperations::SaveFileDialog
                (
                    "",
                    {{ "Trajectory Files (.crsdat)", "*.crsdat" }, { "All Files", "*.*" }}
                );

                if (filename.empty())
                    return;

                std::vector<POINT> trajectoryCopy = trajectory;

                std::thread([trajectoryCopy, filename]()
                {
                    try
                    {
                        std::ofstream file(filename);

                        if (file.is_open())
                        {
                            for (const auto& point : trajectoryCopy)
                                file << point.x << ";" << point.y << "\n";

                            file.close();

                            Logger::GetInstance().InfoF("Trajectory saved to: %s", filename.c_str());
                        }
                        else
                        {
                            Logger::GetInstance().ErrorF("Failed to save trajectory to: %s", filename.c_str());
                        }
                    }
                    catch (const std::exception& e)
                    {
                        Logger::GetInstance().ErrorF("Async save error: %s", e.what());
                    }
                }).detach();
            }

            static void LoadTrajectoryWindowsCtxAsync()
            {
                std::string filename = WinApiFileOperations::OpenFileDialog
                (
                    "",
                    {{ "Trajectory Files (.crsdat)", "*.crsdat" }, { "All Files", "*.*" }}
                );

                if (filename.empty())
                    return;

                std::thread([filename]()
                {
                    try
                    {
                        std::vector<POINT> trajectory;
                        std::ifstream file(filename);
                        std::string line;

                        if (file.is_open())
                        {
                            while (std::getline(file, line))
                            {
                                size_t delimiter = line.find(';');

                                if (delimiter != std::string::npos)
                                {
                                    try
                                    {
                                        POINT point;
                                        point.x = std::stoi(line.substr(0, delimiter));
                                        point.y = std::stoi(line.substr(delimiter + 1));
                                        trajectory.push_back(point);
                                    }
                                    catch (const std::exception& e)
                                    {
                                        Logger::GetInstance().WarningF("Invalid line in trajectory file: %s", line.c_str());
                                    }
                                }
                            }
                            
                            file.close();

                            auto* trajectoryView = dynamic_cast<TrajectoryView*>
                            (
                                ViewRegistry::GetInstance().GetView("TrajectoryView")
                            );
                            
                            if (trajectoryView)
                                trajectoryView->SetTrajectory(trajectory);

                            Logger::GetInstance().InfoF("Trajectory loaded from: %s (%zu points)", 
                                                    filename.c_str(), trajectory.size());
                        }
                        else
                        {
                            Logger::GetInstance().ErrorF("Failed to load trajectory from: %s", filename.c_str());
                        }
                    }
                    catch (const std::exception& e)
                    {
                        Logger::GetInstance().ErrorF("Async load error: %s", e.what());
                    }
                }).detach();
            }

            static void SaveTrajectory(const std::vector<POINT>& trajectory, std::string outputDirectory, std::string filename)
            {
                if (trajectory.empty())
                    return;

                WinApiFileOperations::CreateDirectoryRecursive(outputDirectory);

                std::ofstream file(filename);

                if (file.is_open())
                {
                    for (const auto& point : trajectory)
                        file << point.x << ";" << point.y << "\n";
                    
                    file.close();
                    
                    Logger::GetInstance().InfoF("Trajectory saved to: %s", filename.c_str());
                }
                else
                {
                    Logger::GetInstance().ErrorF("Failed to save trajectory to: %s", filename.c_str());
                }
            }

            static void SaveTrajectoryAsync(const std::vector<POINT>& trajectory, std::string outputDirectory, std::string filename)
            {
                if (trajectory.empty())
                    return;
                
                std::thread([trajectory, outputDirectory, filename]()
                {
                    WinApiFileOperations::CreateDirectoryRecursive(outputDirectory);

                    std::ofstream file(filename);

                    if (file.is_open())
                    {
                        for (const auto& point : trajectory)
                            file << point.x << ";" << point.y << "\n";
                        
                        file.close();
                        
                        Logger::GetInstance().InfoF("Trajectory saved to: %s", filename.c_str());
                    }
                    else
                    {
                        Logger::GetInstance().ErrorF("Failed to save trajectory to: %s", filename.c_str());
                    }
                }).detach();
            }
    };
}

#endif
