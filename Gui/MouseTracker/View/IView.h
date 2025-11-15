#ifndef __MOUSE_TRACKER_IMGUI_IVIEW__
#define __MOUSE_TRACKER_IMGUI_IVIEW__

#include <string>

namespace Mt
{
    struct IView
    {
        protected:
            bool Visible = true;
            
        public:
            virtual ~IView() = default;
            virtual void Draw() = 0;
            virtual const std::string& GetType() const = 0;
            virtual const std::string& GetCategory() const = 0;
            virtual const std::string& GetDisplayName() const = 0;

            virtual bool IsVisible() const
            {
                return Visible;
            }

            virtual void SetVisible(bool visible)
            {
                Visible = visible;
            }
    };
}

#endif
