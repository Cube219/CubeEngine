#pragma once

#include "CoreHeader.h"

#include "GAPI.h"
#include "GAPI_Texture.h"
#include "Matrix.h"
#include "SamplerManager.h"
#include "ShaderManager.h"
#include "ShaderParameter.h"
#include "TextureManager.h"
#include "Vector.h"

namespace cube
{
    class GraphicsPipeline;
    class Material;
    class Mesh;
    class MeshData;
    class Shader;

    namespace platform
    {
        class DLib;
    } // namespace platform

    class GlobalShaderParameters : public ShaderParameters
    {
        CUBE_BEGIN_SHADER_PARAMETERS(GlobalShaderParameters)
            CUBE_SHADER_PARAMETER(Vector3, viewPosition)
            CUBE_SHADER_PARAMETER(Matrix, viewProjection)
            CUBE_SHADER_PARAMETER(Vector3, directionalLightDirection)
        CUBE_END_SHADER_PARAMETERS
    };

    class ObjectShaderParameters : public ShaderParameters
    {
        CUBE_BEGIN_SHADER_PARAMETERS(ObjectShaderParameters)
            CUBE_SHADER_PARAMETER(Matrix, model)
            CUBE_SHADER_PARAMETER(Matrix, modelInverse)
            CUBE_SHADER_PARAMETER(Matrix, modelInverseTranspose)
        CUBE_END_SHADER_PARAMETERS
    };

    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer() = default;

        void Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext, Uint32 numGPUSync = 3);
        void Shutdown(const ImGUIContext& imGUIContext);

        void OnLoopImGUI();
        void RenderAndPresent();

        void OnResize(Uint32 width, Uint32 height);

        GAPI& GetGAPI() const { return *mGAPI; }
        ShaderManager& GetShaderManager() { return mShaderManager; }
        TextureManager& GetTextureManager() { return mTextureManager; }
        SamplerManager& GetSamplerManager() { return mSamplerManager; }
        ShaderParametersManager& GetShaderParametersManager() { return mShaderParametersManager; }

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

        Uint32 mNumGPUSync;
        Uint64 mCurrentRenderingFrame;

        ShaderManager mShaderManager;
        TextureManager mTextureManager;
        SamplerManager mSamplerManager;
        ShaderParametersManager mShaderParametersManager;

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
        SharedPtr<gapi::TextureRTV> mCurrentBackbufferRTV;
        SharedPtr<gapi::Texture> mDepthStencilTexture;
        SharedPtr<gapi::TextureDSV> mDSV;

        Vector3 mDirectionalLightDirection;
        bool mIsDirectionalLightDirty;

        Matrix mModelMatrix;
        SharedPtr<Mesh> mMesh;
        Vector<SharedPtr<Material>> mMaterials;
        SharedPtr<Material> mDefaultMaterial;

        SharedPtr<Mesh> mBoxMesh;
        Matrix mXAxisModelMatrix;
        SharedPtr<Material> mXAxisMaterial;

        Matrix mYAxisModelMatrix;
        SharedPtr<Material> mYAxisMaterial;

        Matrix mZAxisModelMatrix;
        SharedPtr<Material> mZAxisMaterial;
    };
} // namespace cube
