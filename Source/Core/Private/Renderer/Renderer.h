#pragma once

#include "CoreHeader.h"

#include "GAPI.h"
#include "Matrix.h"
#include "Vector.h"

namespace cube
{
    class MeshData;
}

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

    // TODO: Using float version?
    struct alignas(256) GlobalConstantBufferData
    {
        Matrix viewProjection;
    };

    // TODO: Move to renderobject
    struct alignas(256) ObjectBufferData
    {
        Matrix model;
        Vector4 color;
    };

    class Renderer
    {
    public:
        void Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext);
        void Shutdown(const ImGUIContext& imGUIContext);

        void Render();

        void OnResize(Uint32 width, Uint32 height);

        GAPI& GetGAPI() const { return *mGAPI; }

        void SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& upDir);
        void SetPerspectiveMatrix(float fovAngleY, float aspectRatio, float nearZ, float farZ);

    private:
        void SetGlobalConstantBuffers();
        void RenderImpl();

        void LoadResources();
        void ClearResources();

        SharedPtr<platform::DLib> mGAPI_DLib;
        SharedPtr<GAPI> mGAPI;

        Matrix mViewMatrix;
        Matrix mPerspectiveMatrix;
        bool mIsViewPerspectiveMatrixDirty;

        GlobalConstantBufferData mGlobalConstantBufferData;
        SharedPtr<gapi::Buffer> mGlobalConstantBuffer;
        Uint8* mGlobalConstantBufferDataPointer;

        SharedPtr<gapi::CommandList> mCommandList;

        Uint32 mViewportWidth;
        Uint32 mViewportHeight;
        SharedPtr<gapi::Viewport> mViewport;

        SharedPtr<Mesh> mBoxMesh;
        SharedPtr<gapi::Shader> mVertexShader;
        SharedPtr<gapi::Shader> mPixelShader;
        SharedPtr<gapi::ShaderVariablesLayout> mShaderVariablesLayout;
        SharedPtr<gapi::Pipeline> mHelloWorldPipeline;

        ObjectBufferData mObjectBufferData;
        SharedPtr<gapi::Buffer> mObjectBuffer;
        Uint8* mObjectBufferDataPointer;

        ObjectBufferData mObjectBufferData_X;
        SharedPtr<gapi::Buffer> mObjectBuffer_X;
        Uint8* mObjectBufferDataPointer_X;
        ObjectBufferData mObjectBufferData_Y;
        SharedPtr<gapi::Buffer> mObjectBuffer_Y;
        Uint8* mObjectBufferDataPointer_Y;
        ObjectBufferData mObjectBufferData_Z;
        SharedPtr<gapi::Buffer> mObjectBuffer_Z;
        Uint8* mObjectBufferDataPointer_Z;
    };
} // namespace cube
