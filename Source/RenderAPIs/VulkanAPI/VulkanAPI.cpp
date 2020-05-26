#include "VulkanAPI.h"

namespace cube
{
    namespace rapi
    {
        RenderAPI* CreateRenderAPI()
        {
            return new VulkanAPI();
        }

        void VulkanAPI::Initialize()
        {
        }

        void VulkanAPI::Shutdown()
        {
        }
    } // namespace rapi
} // namespace cube
