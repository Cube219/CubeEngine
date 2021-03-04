#pragma once

namespace cube
{
    namespace rapi
    {
        // Interface/BufferVk.h
        class BufferVk;
        class VertexBufferVk;
        class IndexBufferVk;
        class ConstantBufferVk;
        class StructuredBufferVk;

        // Interface/CommandList.h
        class CommandListVk;

        // Interface/FenceVk.h
        class FenceVk;

        // Interface/PipelineStateVk.h
        class GraphicsPipelineStateVk;
        class ComputePipelineStateVk;

        // Interface/RenderPassVk.h
        class RenderPassVk;

        // Interface/ShaderVk.h
        class ShaderVk;

        // Interface/ShaderVariablesVk.h
        class ShaderVariablesVk;
        class ShaderVariablesLayoutVk;

        // Interface/SwapChain.h
        class SwapChainVk;

        // Interface/TextureVk.h
        class TextureVk;
        class Texture2DVk;
        class Texture2DArrayVk;
        class Texture3DVk;
        class TextureCubeVk;

        // VulkanAPI.h
        class VulkanAPI;

        // VulkanCommandPoolManager.h
        struct VulkanCommandBuffer;
        class VulkanCommandPoolManager;

        // VulkanDebug.h
        class VulkanDebug;

        // VulkanDevice.h
        class VulkanDevice;

        // VulkanFencePool.h
        class VulkanFencePool;

        // VulkanLayout.h
        class VulkanLayout;

        // VulkanMemoryAllocator.h
        struct VulkanAllocation;
        class VulkanMemoryAllocator;

        // VulkanQueueManager.h
        class VulkanQueueManager;

        // VulkanSemaphorePool.h
        class VulkanSemaphore;
        class VulkanSemaphorePool;

        // VulkanStagingManager.h
        class VulkanStagingBuffer;
        class VulkanStagingManager;
    } // namespace rapi
} // namespace cube
