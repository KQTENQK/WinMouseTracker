#ifndef __MOUSE_TRACKER_IMGUI_CONSOLEVIEW__
#define __MOUSE_TRACKER_IMGUI_CONSOLEVIEW__

#include "View/IView.h"
#include "Loggers/Logger.h"
#include "Loggers/ImGuiConsoleLogPolicy.h"

namespace Mt
{
    class ConsoleView : public IView
    {
        private:
            bool m_autoScroll;
            
        public:
            ConsoleView()
            {
                m_autoScroll = true;
            }
            
            void Draw() override
            {
                ImGuiConsoleLogPolicy* uiLogPolicy = Logger::GetInstance().GetImGuiConsole();

                if (uiLogPolicy)
                    uiLogPolicy->Draw();
            }
            
            const std::string& GetType() const override 
            { 
                static std::string type = "Console";

                return type; 
            }
            
            const std::string& GetCategory() const override 
            { 
                static std::string category = "Views";

                return category; 
            }
            
            const std::string& GetDisplayName() const override 
            { 
                static std::string name = "Console";

                return name; 
            }
    };
}

#endif
