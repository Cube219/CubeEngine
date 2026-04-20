#include "Renderer.h"

#include "imguizmo_quat/imGuIZMOquat.h"
#include "imgui.h"

#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI_CommandList.h"
#include "GAPI_Pipeline.h"
#include "GAPI_Shader.h"
#include "GAPI_SwapChain.h"
#include "Material.h"
#include "MatrixUtility.h"
#include "MeshHelper.h"
#include "Platform.h"
#include "Renderer/RenderGraphTypes.h"
#include "RenderGraph.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Texture.h"

namespace cube
{
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GlobalShaderParameterList);
    CUBE_REGISTER_SHADER_PARAMETER_LIST(ObjectShaderParameterList);

    Renderer::Renderer()
        : mShaderManager(*this)
        , mTextureManager(*this)
        , mEnvironmentMapping(*this)
    {
    }

    void Renderer::Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext, Uint32 numGPUSync)
    {
        CUBE_LOG(Info, Renderer, "Initialize renderer. (GAPI: {})", GAPINameToString(gAPIName));

        mNumGPUSync = numGPUSync;
        mCurrentRenderingFrame = 0;

        // GAPI init
        platform::FilePath dLibPath;
        switch (gAPIName)
        {
        case GAPIName::DX12:
            dLibPath = platform::FilePath(CUBE_T("CE-GAPI_DX12"));
            break;
        case GAPIName::Metal:
            dLibPath = platform::FilePath(CUBE_T("CE-GAPI_Metal"));
            break;

        case GAPIName::Unknown:
        default:
            CHECK_FORMAT(false, "Invalid GAPIName: {}", (int)gAPIName);
            break;
        }

        mGAPI_DLib = platform::Platform::LoadDLib(dLibPath);
        CHECK_FORMAT(mGAPI_DLib, "Cannot load GAPI library! (Name: {})", dLibPath.ToString());

        using CreateGAPIFunction = GAPI* (*)();
        auto createGAPIFunc = reinterpret_cast<CreateGAPIFunction>(mGAPI_DLib->GetFunction(CUBE_T("CreateGAPI")));
        mGAPI = SharedPtr<GAPI>(createGAPIFunc());

        mGAPI->Initialize({
            .numGPUSync = numGPUSync,
            .enableDebugLayer = true,
            .imGUI = imGUIContext
        });

        mShaderParameterListManager.Initialize(mGAPI.get(), mNumGPUSync);
        mShaderManager.Initialize(mGAPI.get(), false);
        mTextureManager.Initialize(mGAPI.get(), mNumGPUSync);
        mSamplerManager.Initialize(mGAPI.get());

        mRenderImGUI = (imGUIContext.context != nullptr);

        mIsViewPerspectiveMatrixDirty = true;

        mCommandList = mGAPI->CreateCommandList({
            .debugName = CUBE_T("MainCommandList")
        });

        mViewportWidth = platform::Platform::GetWindowWidth();
        mViewportHeight = platform::Platform::GetWindowHeight();
        mSwapChain = mGAPI->CreateSwapChain({
            .width = mViewportWidth,
            .height = mViewportHeight,
            .vsync = true,
            .backbufferCount = 2,
            .debugName = CUBE_T("MainSwapChain")
        });
        mDepthStencilTexture = mGAPI->CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = gapi::ElementFormat::D32_Float,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::DepthStencil,
                .width = mViewportWidth,
                .height = mViewportHeight
            },
            .debugName = CUBE_T("MainDepthStencilTexture")
        });

        mIsDirectionalLightEnabled = true;
        mDirectionalLightDirection = Vector3(1.0f, 1.0f, 1.0f).Normalized();
        mDirectionalLightIntensity = Vector3(1.0f, 1.0f, 1.0f);

        mEnvironmentMapping.Initialize(true);

        LoadResources();
    }

    void Renderer::Shutdown(const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(Info, Renderer, "Shutdown renderer.");

        mGAPI->WaitAllGPUSync();

        ClearResources();

        mEnvironmentMapping.Shutdown();

        mDepthStencilTexture = nullptr;
        mCurrentBackbuffer = nullptr;
        mSwapChain = nullptr;

        mCommandList = nullptr;

        mSamplerManager.Shutdown();
        mTextureManager.Shutdown();
        mShaderManager.Shutdown();
        mShaderParameterListManager.Shutdown();

        mGAPI->Shutdown(imGUIContext);
        mGAPI = nullptr;
        mGAPI_DLib = nullptr;
    }

    void Renderer::OnLoopImGUI()
    {
        ImGui::Begin("Renderer");

        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SeparatorText("Directional Light");
            ImGui::Checkbox("Enable##Directional Light", &mIsDirectionalLightEnabled);
            ImGui::BeginDisabled(!mIsDirectionalLightEnabled);
            {
                vec3 directionVec3 = { mDirectionalLightDirection.GetFloat3().x, mDirectionalLightDirection.GetFloat3().y, mDirectionalLightDirection.GetFloat3().z };
                ImGui::Text("Direction: %.3f %.3f %.3f", directionVec3.x, directionVec3.y, directionVec3.z);

                Vector4 directionInView = Vector4(mDirectionalLightDirection) * mViewMatrix;
                vec3 directionInViewVec3 = { directionInView.GetFloat3().x, directionInView.GetFloat3().y, directionInView.GetFloat3().z };

                imguiGizmo::resizeAxesOf({ 0.7f, 0.8f, 0.8f });
                ImGui::gizmo3D("##Directional Light - Direction", directionInViewVec3);
                imguiGizmo::restoreAxesSize();

                directionInView = Vector4(directionInViewVec3.x, directionInViewVec3.y, directionInViewVec3.z, 0);
                Vector4 afterDirection = directionInView * mViewMatrix.Inversed();

                mDirectionalLightDirection = Vector3(afterDirection);
            }
            {
                Float3 intensityFloat3 = mDirectionalLightIntensity.GetFloat3();
                ImGui::DragFloat3("Intensity", &intensityFloat3.x, 0.1f);

                mDirectionalLightIntensity = Vector3(intensityFloat3.x, intensityFloat3.y, intensityFloat3.z);
            }
            ImGui::EndDisabled();

            mEnvironmentMapping.OnLoopImGUI();
        }

        if (ImGui::CollapsingHeader("Shader", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Shader Debug Mode", &mShaderManager.mUseDebugMode))
            {
                mShaderManager.RecompileShaders(true);
            }

            if (ImGui::Button("RecompileShader"))
            {
                mShaderManager.RecompileShaders(false);
            }
        }

        if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SeparatorText("Gizmos");

            ImGui::Checkbox("Show Axis", &mShowAxis);
            if (ImGui::Checkbox("Wireframe", &mWireframe))
            {
                mNeedRecreatingPipelines = true;
            }
        }

        ImGui::End();
    }

    void Renderer::RenderAndPresent()
    {
        mCurrentRenderingFrame++;
        mGAPI->BeginRenderingFrame();
        mShaderParameterListManager.MoveNextFrame();

        SetGlobalConstantBuffers();

        mSwapChain->AcquireNextImage();
        mCurrentBackbuffer = mSwapChain->GetCurrentBackbuffer();
        mCommandList->Reset();
        {
            mCommandList->Begin();
            
            mCommandList->ResourceTransition({
                .resourceType = gapi::TransitionState::ResourceType::Texture,
                .texture = mCurrentBackbuffer,
                .subresourceRange = {
                    .firstMipLevel = 0,
                    .mipLevels = 1,
                    .firstSliceIndex = 0,
                    .sliceSize = 1
                },
                .src = gapi::ResourceStateFlag::Present,
                .dst = gapi::ResourceStateFlag::Common
            });

            mCommandList->End();
            mCommandList->Submit();
        }

        mGAPI->OnBeforeRender();

        RenderImpl();

        mGAPI->OnAfterRender();

        if (mRenderImGUI)
        {
            ImGui::Render();
        }

        mGAPI->OnBeforePresent(mCurrentBackbuffer.get());
        mCommandList->Reset();
        {
            mCommandList->Begin();

            mCommandList->ResourceTransition({
                .resourceType = gapi::TransitionState::ResourceType::Texture,
                .texture = mCurrentBackbuffer,
                .subresourceRange = {
                    .firstMipLevel = 0,
                    .mipLevels = 1,
                    .firstSliceIndex = 0,
                    .sliceSize = 1
                },
                .src = gapi::ResourceStateFlag::RenderTarget,
                .dst = gapi::ResourceStateFlag::Present
            });

            mCommandList->End();
            mCommandList->Submit();
        }
        mSwapChain->Present();
        mGAPI->OnAfterPresent();

        mGAPI->EndRenderingFrame();
    }

    void Renderer::OnResize(Uint32 width, Uint32 height)
    {
        mViewportWidth = width;
        mViewportHeight = height;

        mGAPI->WaitAllGPUSync();

        mCurrentBackbuffer = nullptr;
        if (mSwapChain)
        {
            mSwapChain->Resize(width, height);
        }

        if (mDepthStencilTexture)
        {
            mDepthStencilTexture = nullptr;

            mDepthStencilTexture = mGAPI->CreateTexture({
                .usage = gapi::ResourceUsage::GPUOnly,
                .textureInfo = {
                    .format = gapi::ElementFormat::D32_Float,
                    .type = gapi::TextureType::Texture2D,
                    .flags = gapi::TextureFlag::DepthStencil,
                    .width = mViewportWidth,
                    .height = mViewportHeight
                },
                .debugName = CUBE_T("MainDepthStencilTexture")
            });
        }
    }

    void Renderer::SetObjectModelMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale)
    {
        mModelMatrix = MatrixUtility::GetScale(scale) * MatrixUtility::GetRotationXYZ(rotation) + MatrixUtility::GetTranslation_Add(position);
    }

    void Renderer::SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& upDir)
    {
        mViewPosition = eye;
        mViewMatrix = MatrixUtility::GetLookAt(eye, target, upDir);
        mIsViewPerspectiveMatrixDirty = true;
    }

    void Renderer::SetPerspectiveMatrix(float fovAngleY, float aspectRatio, float nearZ, float farZ)
    {
        mPerspectiveMatrix = MatrixUtility::GetPerspectiveFov(fovAngleY, aspectRatio, nearZ, farZ);
        mIsViewPerspectiveMatrixDirty = true;
    }

    float Renderer::GetGPUTimeMS() const
    {
        gapi::TimestampList timestampList = mGAPI->GetLastTimestampList();

        const Vector<gapi::Timestamp>& timestamps = timestampList.timestamps;
        if (timestamps.size() >= 2)
        {
            const Uint64 gpuTime = (timestamps.back().time - timestamps[0].time);
            return static_cast<float>(static_cast<double>(gpuTime) / static_cast<double>(timestampList.frequency) * 1000.0);
        }
        else
        {
            return 0.0f;
        }
    }

    void Renderer::SetMesh(SharedPtr<MeshData> meshData, const MeshMetadata& meshMeta)
    {
        bool metaChanged = (mMeshMetadata.useFloat16 != meshMeta.useFloat16);
        mMeshMetadata = meshMeta;

        if (meshData)
        {
            mMesh = std::make_shared<Mesh>(meshData, mMeshMetadata);
        }
        else
        {
            mMesh = std::make_shared<Mesh>(MeshHelper::GenerateBoxMeshData(), mMeshMetadata);
        }

        if (metaChanged)
        {
            mBoxMesh = std::make_shared<Mesh>(MeshHelper::GenerateBoxMeshData(), mMeshMetadata);
            mShaderManager.GetMaterialShaderManager().ClearPipelineCache();

            mNeedRecreatingPipelines = true;
        }
    }

    void Renderer::SetMaterials(const Vector<SharedPtr<Material>>& materials)
    {
        mMaterials = materials;
    }

    void Renderer::SetGlobalConstantBuffers()
    {
        if (mIsViewPerspectiveMatrixDirty)
        {
            mViewPerspectiveMatirx = mViewMatrix * mPerspectiveMatrix;
            mIsViewPerspectiveMatrixDirty = false;
        }
    }

    void Renderer::RenderImpl()
    {
        RecreatePipelinesIfNeeded();

        gapi::RasterizerState::FillMode fillMode = mWireframe
            ? gapi::RasterizerState::FillMode::Line
            : gapi::RasterizerState::FillMode::Solid;

        RGBuilder builder(*this);

        {
            RG_GPU_EVENT_SCOPE(builder, CUBE_T("Frame"));

            gapi::Viewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(mViewportWidth),
                .height = static_cast<float>(mViewportHeight)
            };
            gapi::ScissorRect scissor = {
                .x = 0,
                .y = 0,
                .width = mViewportWidth,
                .height = mViewportHeight
            };

            RGShaderParameterListHandle<GlobalShaderParameterList> globalShaderParameterList = builder.CreateShaderParameterList<GlobalShaderParameterList>();
            globalShaderParameterList->Get()->viewPosition = mViewPosition;
            globalShaderParameterList->Get()->viewProjection = mViewPerspectiveMatirx;
            globalShaderParameterList->Get()->isDirectionalLightEnabled = mIsDirectionalLightEnabled;
            globalShaderParameterList->Get()->directionalLightDirection = mDirectionalLightDirection;
            globalShaderParameterList->Get()->directionalLightIntensity = mDirectionalLightIntensity;
            globalShaderParameterList->Get()->WriteAllParametersToGPUBuffer();
            builder.BindShaderParameterList(globalShaderParameterList);

            RGShaderParameterListHandle<EnvironmentMapLightShaderParameterList> envMapShaderParameterList = builder.CreateShaderParameterList<EnvironmentMapLightShaderParameterList>();
            envMapShaderParameterList->Get()->diffuseIrradianceMap = mEnvironmentMapping.GetDiffuseIrradianceMap(builder);
            envMapShaderParameterList->Get()->sampler.id = mSamplerManager.GetDefaultLinearSamplerId();
            envMapShaderParameterList->Get()->WriteAllParametersToGPUBuffer();

            RGTextureHandle color = builder.RegisterTexture(mCurrentBackbuffer);
            RGTextureHandle depthStencil = builder.RegisterTexture(mDepthStencilTexture);

            RGTextureRTVHandle colorRTV = builder.CreateRTV(color);
            RGTextureDSVHandle depthStencilDSV = builder.CreateDSV(depthStencil);

            RGBuilder::RenderPassInfo renderPassInfo;
            renderPassInfo.colors.push_back({
                .color = colorRTV,
                .loadOperation = gapi::LoadOperation::Clear,
                .storeOperation = gapi::StoreOperation::Store,
                .clearColor = { 0.2f, 0.2f, 0.2f, 1.0f }
            });
            renderPassInfo.depthstencil = {
                .dsv = depthStencilDSV,
                .loadOperation = gapi::LoadOperation::Clear,
                .storeOperation = gapi::StoreOperation::Store,
                .clearDepth = 0.0f
            };
            builder.BeginRenderPass(renderPassInfo);

            builder.AddPass(CUBE_T("Init global settings"), [viewport, scissor](gapi::CommandList& commandList)
            {
                gapi::Viewport vp = viewport;
                gapi::ScissorRect sr = scissor;
                commandList.SetViewports({ &vp, 1 });
                commandList.SetScissors({ &sr, 1 });
                commandList.SetPrimitiveTopology(gapi::PrimitiveTopology::TriangleList);
            });

            FrameVector<RGBuilder::DrawMeshInfo> drawMeshInfos;
            drawMeshInfos.push_back({
                .mesh = mMesh,
                .meshMetaData = mMeshMetadata,
                .fillMode = fillMode,
                .materials = mMaterials,
                .model = mModelMatrix
            });
            builder.AddDrawMeshPass(CUBE_T("Draw Center Object"), drawMeshInfos, RGBuilder::MakeParameterListArray(envMapShaderParameterList));

            if (mShowAxis)
            {
                FrameVector<RGBuilder::DrawMeshInfo> drawAxisMeshInfos;
                drawAxisMeshInfos.push_back({
                    .mesh = mBoxMesh,
                    .meshMetaData = mMeshMetadata,
                    .fillMode = fillMode,
                    .materials = { &mXAxisMaterial, 1 },
                    .model = mXAxisModelMatrix
                });
                drawAxisMeshInfos.push_back({
                    .mesh = mBoxMesh,
                    .meshMetaData = mMeshMetadata,
                    .fillMode = fillMode,
                    .materials = { &mYAxisMaterial, 1 },
                    .model = mYAxisModelMatrix
                });
                drawAxisMeshInfos.push_back({
                    .mesh = mBoxMesh,
                    .meshMetaData = mMeshMetadata,
                    .fillMode = fillMode,
                    .materials = { &mZAxisMaterial, 1 },
                    .model = mZAxisModelMatrix
                });
                builder.AddDrawMeshPass(CUBE_T("Draw Axis"), drawAxisMeshInfos, RGBuilder::MakeParameterListArray(envMapShaderParameterList));
            }

            mEnvironmentMapping.DrawSkybox(builder);

            builder.EndRenderPass();
        }
        builder.ExecuteAndSubmit(*mCommandList);
    }

    void Renderer::LoadResources()
    {
        mBoxMesh = std::make_shared<Mesh>(MeshHelper::GenerateBoxMeshData(), mMeshMetadata);
        SetMesh(nullptr, mMeshMetadata); // Load default mesh

        mDefaultMaterial = std::make_shared<Material>(CUBE_T("DefaultMaterial"));

        {
            Array<Uint32, 6> dummyValue;
            memset(dummyValue.data(), 0, sizeof(Uint32) * dummyValue.size());
            TextureResourceCreateInfo dummyTextureCreateInfo = {
                .textureInfo = {
                    .format = gapi::ElementFormat::RGBA8_UNorm,
                    .type = gapi::TextureType::Texture2D,
                    .width = 1,
                    .height = 1,
                },
                .data = BlobView(dummyValue.data(), sizeof(Uint32)),
                .bytesPerElement = sizeof(Uint32),
                .debugName = CUBE_T("DummyBlackTexture2D")
            };
            mDummyBlackTexture2D = std::make_shared<TextureResource>(dummyTextureCreateInfo);
            dummyTextureCreateInfo.textureInfo.type  = gapi::TextureType::TextureCube;
            dummyTextureCreateInfo.data = BlobView(dummyValue.data(), sizeof(Uint32) * 6);
            dummyTextureCreateInfo.debugName = CUBE_T("DummyBlackTextureCube");
            mDummyBlackTextureCube = std::make_shared<TextureResource>(dummyTextureCreateInfo);

            memset(dummyValue.data(), 0xFF, sizeof(Uint32) * dummyValue.size());
            dummyTextureCreateInfo.textureInfo.type  = gapi::TextureType::Texture2D;
            dummyTextureCreateInfo.data = BlobView(dummyValue.data(), sizeof(Uint32));
            dummyTextureCreateInfo.debugName = CUBE_T("DummyWhiteTexture2D");
            mDummyWhiteTexture2D = std::make_shared<TextureResource>(dummyTextureCreateInfo);
        }

        {
            mXAxisModelMatrix = MatrixUtility::GetScale(4.0f, 0.2f, 0.2f) + MatrixUtility::GetTranslation_Add(2, 0, 0);
            mXAxisMaterial = std::make_shared<Material>(CUBE_T("XAxisMaterial"));
            mXAxisMaterial->SetBaseColor({ 1.0f, 0.0f, 0.0f, 1.0f });

            mYAxisModelMatrix = MatrixUtility::GetScale(0.2f, 4.0f, 0.2f) + MatrixUtility::GetTranslation_Add(0, 2, 0);
            mYAxisMaterial = std::make_shared<Material>(CUBE_T("YAxisMaterial"));
            mYAxisMaterial->SetBaseColor({ 0.0f, 1.0f, 0.0f, 1.0f });

            mZAxisModelMatrix = MatrixUtility::GetScale(0.2f, 0.2f, 4.0f) + MatrixUtility::GetTranslation_Add(0, 0, 2);
            mZAxisMaterial = std::make_shared<Material>(CUBE_T("ZAxisMaterial"));
            mZAxisMaterial->SetBaseColor({ 0.0f, 0.0f, 1.0f, 1.0f });
        }

        mEnvironmentMapping.LoadResources();

        mNeedRecreatingPipelines = true;
        RecreatePipelinesIfNeeded();
    }

    void Renderer::ClearResources()
    {
        mEnvironmentMapping.ClearReaources();

        mZAxisMaterial = nullptr;
        mYAxisMaterial = nullptr;
        mXAxisMaterial = nullptr;

        mDummyWhiteTexture2D = nullptr;
        mDummyBlackTextureCube = nullptr;
        mDummyBlackTexture2D = nullptr;
        mDefaultMaterial = nullptr;

        mMaterials.clear();
        mMesh = nullptr;
        mBoxMesh = nullptr;
    }

    void Renderer::RecreatePipelinesIfNeeded()
    {
        if (!mNeedRecreatingPipelines)
        {
            return;
        }

        mEnvironmentMapping.RecreateGraphicsPipelines();

        mNeedRecreatingPipelines = false;
    }
} // namespace cube
