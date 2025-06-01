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
        Vector3 directionalLightDirection;
    };

    // TODO: Move to renderobject
    struct alignas(256) ObjectBufferData
    {
        Matrix model;
        Matrix modelInverse;
        Vector4 color;

        void SetModelMatrix(const Matrix& newModel)
        {
            model = newModel;
            modelInverse = model.Inversed();
        }
    };

    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer() = default;

        void Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext);
        void Shutdown(const ImGUIContext& imGUIContext);

        void OnLoopImGUI();
        void Render();

        void OnResize(Uint32 width, Uint32 height);

        GAPI& GetGAPI() const { return *mGAPI; }

        void SetObjectModelMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale);
        void SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& upDir);
        void SetPerspectiveMatrix(float fovAngleY, float aspectRatio, float nearZ, float farZ);

        float GetGPUTimeMS() const;

        void SetMesh(SharedPtr<MeshData> meshData);

    private:
        void SetGlobalConstantBuffers();
        void RenderImpl();

        void LoadResources();
        void ClearResources();

        SharedPtr<platform::DLib> mGAPI_DLib;
        SharedPtr<GAPI> mGAPI;
        bool mRenderImGUI;

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

        Vector3 mDirectionalLightDirection;
        bool mIsDirectionalLightDirty;

        SharedPtr<Mesh> mMesh;
        SharedPtr<gapi::Shader> mVertexShader;
        SharedPtr<gapi::Shader> mPixelShader;
        SharedPtr<gapi::ShaderVariablesLayout> mShaderVariablesLayout;
        SharedPtr<gapi::Pipeline> mMainPipeline;

        ObjectBufferData mObjectBufferData;
        SharedPtr<gapi::Buffer> mObjectBuffer;
        Uint8* mObjectBufferDataPointer;

        SharedPtr<Mesh> mBoxMesh;
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
