#pragma once

namespace cube
{
    namespace rapi
    {
        // Interface/BufferVk.h
        class BufferVk;
        class VertexBufferVk;
        class IndexBufferVk;

        // Interface/DeviceContextVk.h
        class DeviceContextVk;

        // Interface/TextureVk.h
        class TextureVk;
        class Texture2DVk;
        class Texture2DArrayVk;
        class Texture3DVk;
        class TextureCubeVk;

        // VulkanAPI.h
        class VulkanAPI;

        // VulkanCommandPool.h
        class VulkanCommandBuffer;
        class VulkanCommandPool;

        // VulkanDebug.h
        class VulkanDebug;

        // VulkanDevice.h
        class VulkanDevice;

        // VulkanFencePool.h
        class VulkanFence;
        class VulkanFencePool;

        // VulkanMemoryAllocator.h
        struct VulkanAllocation;
        class VulkanMemoryAllocator;

        // VulkanQueueManager.h
        class VulkanQueueManager;

        // VulkanSemaphoreManager.h
        class VulkanSemaphore;
        class VulkanSemaphoreManager;

        // VulkanStagingManager.h
        class VulkanStagingBuffer;
        class VulkanStagingManager;
    } // namespace rapi
} // namespace cube
