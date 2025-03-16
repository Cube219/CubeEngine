#pragma once

#include "CoreHeader.h"

#include "GAPI.h"

namespace cube
{
    namespace gapi {
        class Pipeline;
        class Shader;
    } // namespace gapi

    class Mesh;

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

        GAPI& GetGAPI() const { return *mGAPI; }

    private:
        void RenderImpl();

        void LoadResources();
        void ClearResources();

        SharedPtr<platform::DLib> mGAPI_DLib;
        SharedPtr<GAPI> mGAPI;

        SharedPtr<gapi::CommandList> mCommandList;

        Uint32 mViewportWidth;
        Uint32 mViewportHeight;
        SharedPtr<gapi::Viewport> mViewport;

        SharedPtr<gapi::Buffer> mTriangleVertexBuffer;
        SharedPtr<gapi::Shader> mVertexShader;
        SharedPtr<gapi::Shader> mPixelShader;
        SharedPtr<gapi::ShaderVariablesLayout> mEmptyShaderVariablesLayout;
        SharedPtr<gapi::Pipeline> mHelloWorldPipeline;
    };
} // namespace cube
