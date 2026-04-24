#pragma once

#include "CoreHeader.h"

#include "DLib.h"
#include "EnvironmentMapping.h"
#include "GAPI.h"
#include "GAPI_Texture.h"
#include "Matrix.h"
#include "Pipeline.h"
#include "Renderer/Mesh.h"
#include "Renderer/ShaderParameter.h"
#include "SamplerManager.h"
#include "Shader.h"
#include "Texture.h"
#include "TextureManager.h"
#include "TextureViewer.h"
#include "Vector.h"

namespace cube
{
    class GraphicsPipeline;
    class Material;
    class MeshData;
    class Shader;

    class GlobalShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GlobalShaderParameterList)
            CUBE_SHADER_PARAMETER(Vector3, viewPosition)
            CUBE_SHADER_PARAMETER(Matrix, viewProjection)
            CUBE_SHADER_PARAMETER(bool, isDirectionalLightEnabled)
            CUBE_SHADER_PARAMETER(Vector3, directionalLightDirection)
            CUBE_SHADER_PARAMETER(Vector3, directionalLightIntensity)
        CUBE_END_SHADER_PARAMETER_LIST
    };

    class ObjectShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(ObjectShaderParameterList)
            CUBE_SHADER_PARAMETER(Matrix, model)
            CUBE_SHADER_PARAMETER(Matrix, modelInverse)
            CUBE_SHADER_PARAMETER(Matrix, modelInverseTranspose)
        CUBE_END_SHADER_PARAMETER_LIST
    };

    class Renderer
    {
    public:
        Renderer();
        ~Renderer() = default;

        void Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext, Uint32 numGPUSync = 3);
        void Shutdown(const ImGUIContext& imGUIContext);

        void OnLoopImGUI();
        void RenderAndPresent();

        void OnResize(Uint32 width, Uint32 height);

        GAPI& GetGAPI() const { return *mGAPI; }
        ShaderParameterListManager& GetShaderParameterListManager() { return mShaderParameterListManager; }
        ShaderManager& GetShaderManager() { return mShaderManager; }
        TextureManager& GetTextureManager() { return mTextureManager; }
        SamplerManager& GetSamplerManager() { return mSamplerManager; }
        PipelineManager& GetPipelineManager() { return mPipelineManager; }

        TextureViewer& GetTextureViewer() { return mTextureViewer; }

        bool IsDrawInWireframe() const { return mWireframe; }

        SharedPtr<Mesh> GetBoxMesh() const { return mBoxMesh; }
        SharedPtr<Material> GetDefaultMaterial() const { return mDefaultMaterial; }
        SharedPtr<gapi::Texture> GetDummyBlackTexture2D() const { return mDummyBlackTexture2D->GetGAPITexture(); }
        SharedPtr<gapi::Texture> GetDummyBlackTextureCube() const { return mDummyBlackTextureCube->GetGAPITexture(); }
        SharedPtr<gapi::Texture> GetDummyWhiteTexture2D() const { return mDummyWhiteTexture2D->GetGAPITexture(); }

        void SetObjectModelMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale);
        void SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& upDir);
        void SetPerspectiveMatrix(float fovAngleY, float aspectRatio, float nearZ, float farZ);

        float GetGPUTimeMS() const;
        Uint64 GetCurrentRenderingFrame() const { return mCurrentRenderingFrame; };

        const MeshMetadata& GetMeshMetadata() const { return mMeshMetadata; }
        void SetMesh(SharedPtr<MeshData> meshData, const MeshMetadata& meshMeta);
        void SetMaterials(const Vector<SharedPtr<Material>>& materials);

    private:
        void SetGlobalConstantBuffers();
        void RenderImpl();

        void LoadResources();
        void ClearResources();

        SharedPtr<platform::DLib> mGAPI_DLib;
        SharedPtr<GAPI> mGAPI;

        Uint32 mNumGPUSync;
        Uint64 mCurrentRenderingFrame;

        ShaderParameterListManager mShaderParameterListManager;
        ShaderManager mShaderManager;
        TextureManager mTextureManager;
        SamplerManager mSamplerManager;
        PipelineManager mPipelineManager;

        EnvironmentMapping mEnvironmentMapping;

        TextureViewer mTextureViewer;

        bool mRenderImGUI;

        Vector3 mViewPosition;
        Matrix mViewMatrix;
        Matrix mPerspectiveMatrix;
        Matrix mViewPerspectiveMatirx;
        bool mIsViewPerspectiveMatrixDirty;

        SharedPtr<gapi::CommandList> mCommandList;

        Uint32 mViewportWidth;
        Uint32 mViewportHeight;
        SharedPtr<gapi::SwapChain> mSwapChain;
        SharedPtr<gapi::Texture> mCurrentBackbuffer;
        SharedPtr<gapi::Texture> mDepthStencilTexture;

        bool mIsDirectionalLightEnabled;
        Vector3 mDirectionalLightDirection;
        Vector3 mDirectionalLightIntensity;

        MeshMetadata mMeshMetadata;
        Matrix mModelMatrix;
        SharedPtr<Mesh> mMesh;
        Vector<SharedPtr<Material>> mMaterials;
        SharedPtr<Material> mDefaultMaterial;

        SharedPtr<TextureResource> mDummyBlackTexture2D;
        SharedPtr<TextureResource> mDummyWhiteTexture2D;
        SharedPtr<TextureResource> mDummyBlackTextureCube;

        bool mShowAxis = true;
        bool mWireframe = false;
        SharedPtr<Mesh> mBoxMesh;
        Matrix mXAxisModelMatrix;
        SharedPtr<Material> mXAxisMaterial;

        Matrix mYAxisModelMatrix;
        SharedPtr<Material> mYAxisMaterial;

        Matrix mZAxisModelMatrix;
        SharedPtr<Material> mZAxisMaterial;
    };
} // namespace cube
