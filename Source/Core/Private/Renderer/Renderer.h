#pragma once

#include "CoreHeader.h"

#include "GAPI.h"
#include "Matrix.h"
#include "SamplerManager.h"
#include "Vector.h"

namespace cube
{
    namespace gapi {
        class Pipeline;
        class Shader;
    } // namespace gapi

    class Material;
    class Mesh;
    class MeshData;

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
        SamplerManager& GetSamplerManager() { return mSamplerManager; }

        void SetObjectModelMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale);
        void SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& upDir);
        void SetPerspectiveMatrix(float fovAngleY, float aspectRatio, float nearZ, float farZ);

        float GetGPUTimeMS() const;

        void SetMesh(SharedPtr<MeshData> meshData);
        void SetMaterials(const Vector<SharedPtr<Material>>& materials);

    private:
        void SetGlobalConstantBuffers();
        void RenderImpl();

        void LoadResources();
        void ClearResources();

        SharedPtr<platform::DLib> mGAPI_DLib;
        SharedPtr<GAPI> mGAPI;

        SamplerManager mSamplerManager;

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
        Vector<SharedPtr<Material>> mMaterials;
        SharedPtr<gapi::Shader> mVertexShader;
        SharedPtr<gapi::Shader> mPixelShader;
        SharedPtr<gapi::ShaderVariablesLayout> mShaderVariablesLayout;
        SharedPtr<gapi::Pipeline> mMainPipeline;

        ObjectBufferData mObjectBufferData;
        SharedPtr<gapi::Buffer> mObjectBuffer;
        Uint8* mObjectBufferDataPointer;

        SharedPtr<Material> mDefaultMaterial;

        SharedPtr<Mesh> mBoxMesh;
        ObjectBufferData mXAxisObjectBufferData;
        SharedPtr<gapi::Buffer> mXAxisObjectBuffer;
        Uint8* mXAxisObjectBufferDataPointer;
        SharedPtr<Material> mXAxisMaterial;

        ObjectBufferData mYAxisObjectBufferData;
        SharedPtr<gapi::Buffer> mYAxisObjectBuffer;
        Uint8* mYAxisObjectBufferDataPointer;
        SharedPtr<Material> mYAxisMaterial;

        ObjectBufferData mZAxisObjectBufferData;
        SharedPtr<gapi::Buffer> mZAxisObjectBuffer;
        Uint8* mZAxisObjectBufferDataPointer;
        SharedPtr<Material> mZAxisMaterial;
    };
} // namespace cube
