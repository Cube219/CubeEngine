#pragma once

#include "VulkanAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        class VulkanStagingBuffer
        {
        };

        class VulkanStagingManager
        {
        public:
            VulkanStagingManager() {}
            ~VulkanStagingManager() {}

            void Initialize();
            void Shutdown();

        private:
            struct StagingBufferInfo
            {
                VulkanStagingBuffer* pBuf;
            };

            Vector<StagingBufferInfo> mFreeStagingBuffers;
        };
    } // namespace rapi
} // namespace cube
