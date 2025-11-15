#ifndef __MOUSE_TRACKER_IMGUI_VIEWREGISTRY__
#define __MOUSE_TRACKER_IMGUI_VIEWREGISTRY__ 

#include <unordered_map>
#include <vector>
#include <string>
#include "IView.h"
#include "ViewRegistration.h"

namespace Mt
{
    class ViewRegistry
    {
        private:
            std::unordered_map<std::string, ViewRegistration> m_registrations;
            std::unordered_map<std::string, std::unique_ptr<IView>> m_instances;
            std::vector<std::string> m_viewOrder;

        public:
            static ViewRegistry& GetInstance()
            {
                static ViewRegistry instance;

                return instance;
            }
            
            template<typename TView>
            void RegisterView
            (
                const std::string& type,
                const std::string& category,
                const std::string& displayName,
                bool enabled = true
            )
            {
                m_registrations[type] = ViewRegistration
                {
                    type, category, displayName, 
                    []() { return std::make_unique<TView>(); },
                    enabled
                };
                m_viewOrder.push_back(type);
            }

            void CreateViewInstance(const std::string& type)
            {
                if (m_instances.find(type) != m_instances.end())
                    return;
        
                auto it = m_registrations.find(type);

                if (it != m_registrations.end())
                    m_instances[type] = it->second.Instantiate();
            }

            IView* GetView(const std::string& type)
            {
                CreateViewInstance(type);
                auto it = m_instances.find(type);

                return it != m_instances.end() ? it->second.get() : nullptr;
            }

            std::vector<std::string> GetViewTypesByCategory(const std::string& category) const
            {
                std::vector<std::string> result;

                for (const auto& [type, reg] : m_registrations)
                    if (reg.Category == category)
                        result.push_back(type);
                
                return result;
            }

            const std::vector<std::string>& GetViewOrder() const
            {
                return m_viewOrder;
            }

            void InitializeAllViews()
            {
                for (const auto& type : m_viewOrder)
                    CreateViewInstance(type);
            }
    };
}

#endif
