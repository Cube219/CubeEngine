#pragma once

#include "CoreHeader.h"

#include "GAPI.h"

namespace cube
{

    namespace platform
    {
        class DLib;
    } // namespace platform

    class Renderer
    {
    public:
        void Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext);
        void Shutdown(const ImGUIContext& imGUIContext);

        void Render();

        void OnResize(Uint32 width, Uint32 height);

    private:
        SharedPtr<platform::DLib> mGAPI_DLib;
        SharedPtr<GAPI> mGAPI;

        SharedPtr<gapi::Viewport> mViewport;
    };
} // namespace cube
