#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class LoadOperator
        {
            Load,
            Clear,
            DontCare
        };

        enum class StoreOperator
        {
            Store,
            DontCare
        };

        struct RenderPassAttachmentInfo
        {
            TextureFormat format = TextureFormat::Unknown;
            Uint8 sampleCount = 1;
            LoadOperator loadOp = LoadOperator::Load;
            StoreOperator storeOp = StoreOperator::Store;
            LoadOperator stencilLoadOp = LoadOperator::Load;
            StoreOperator stencilStoreOp = StoreOperator::Store;
        };

        struct Subpass
        {
            Uint32 numInputAttachments = 0;
            Uint32* inputAttachmentIndices = nullptr;

            Uint32 numColorAttachments = 0;
            Uint32* colorAttachmentIndices = nullptr;

            Int32 depthStencilAttachmentIndex = -1;
        };

        struct SubpassDependency
        {
            Uint32 srcIndex;
            Uint32 dstIndex;
        };

        struct RenderPassCreateInfo
        {
            Uint32 numAttachments = 0;
            RenderPassAttachmentInfo* attachments = nullptr;

            Uint32 numSubpasses = 0;
            Subpass* subpasses = nullptr;

            Uint32 numDependencies = 0;
            SubpassDependency* dependencies = nullptr;

            const char* debugName = "";
        };

        class RenderPass
        {
        public:
            RenderPass() = default;
            virtual ~RenderPass() = default;
        };
    } // namespace rapi
} // namespace cube
