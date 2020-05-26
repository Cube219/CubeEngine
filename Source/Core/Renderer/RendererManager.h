#pragma once

#include "../CoreHeader.h"

#include "Platform/DLib.h"
#include "RenderAPIs/RenderAPI/RenderAPI.h"

namespace cube
{
    enum class RenderAPIType
    {
        Vulkan
    };

    class CORE_EXPORT RendererManager
    {
    public:
        RendererManager() = delete;
        ~RendererManager() = delete;

        RendererManager(const RendererManager& other) = delete;
        RendererManager& operator=(const RendererManager& rhs) = delete;

        static void Initialize(RenderAPIType apiType);
        static void Shutdown();

        static void Render();

        static void Resize(Uint32 width, Uint32 height);

    private:
        static SPtr<platform::DLib> mRenderAPIDLib;
        static SPtr<rapi::RenderAPI> mRenderAPI;
    };
} // namespace cube
