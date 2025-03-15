#pragma once

#include "CubeString.h"

#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        class CommandList;
        struct CommandListCreateInfo;
        class Fence;
        struct FenceCreateInfo;
        class Viewport;
        struct ViewportCreateInfo;
    }

    enum class GAPIName
    {
        Unknown,
        DX12
    };
    inline const Character* GAPINameToString(GAPIName gAPIName)
    {
        switch (gAPIName)
        {
        case GAPIName::DX12:
            return CUBE_T("DX12");
        case GAPIName::Unknown:
        default:
            return CUBE_T("Unknown");
        }
    }

    struct ImGUIContext
    {
        void* context = nullptr;
        void* allocFunc = nullptr;
        void* freeFunc = nullptr;
        void* userData = nullptr;
    };

    struct GAPIInitInfo
    {
        bool enableDebugLayer = false;
        ImGUIContext imGUI;
    };

    class GAPI
    {
    public:
        GAPI() = default;
        virtual ~GAPI() = default;

        virtual void Initialize(const GAPIInitInfo& initInfo) = 0;
        virtual void Shutdown(const ImGUIContext& imGUIInfo) = 0;

        virtual void OnBeforeRender() = 0;
        virtual void OnAfterRender() = 0;
        virtual void OnBeforePresent(gapi::Viewport* viewport) = 0;
        virtual void OnAfterPresent() = 0;

        GAPIName GetAPIName() const { return mAPIName; }

        virtual SharedPtr<gapi::CommandList> CreateCommandList(const gapi::CommandListCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Fence> CreateFence(const gapi::FenceCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Viewport> CreateViewport(const gapi::ViewportCreateInfo& info) = 0;

    protected:
        GAPIName mAPIName = GAPIName::Unknown;
    };
} // namespace cube
