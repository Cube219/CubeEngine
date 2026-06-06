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
#include "Texture.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

namespace cube
{
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GlobalShaderParameterList);
    CUBE_REGISTER_SHADER_PARAMETER_LIST(ObjectShaderParameterList);
    CUBE_REGISTER_SHADER_PARAMETER_LIST(SubMeshShaderParameterList);

    Renderer::Renderer()
        : mShaderManager(*this)
        , mTextureManager(*this)
        , mPipelineManager(*this)
        , mEnvironmentMapping(*this)
        , mTonemap(*this)
        , mRenderUtils(*this)
        , mTextureViewer(*this)
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

        mBackbufferFormat = gapi::ElementFormat::RGBA8_UNorm;
        mColorFormat = gapi::ElementFormat::RG11B10_Float;
        mDepthStencilFormat = gapi::ElementFormat::D32_Float;

        mShaderParameterListManager.Initialize(mGAPI.get(), mNumGPUSync);
        mShaderManager.Initialize(false);
        mTextureManager.Initialize(mGAPI.get(), mNumGPUSync);
        mSamplerManager.Initialize(mGAPI.get());
        mPipelineManager.Initialize();

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
            .backbufferFormat = mBackbufferFormat,
            .backbufferCount = 2,
            .debugName = CUBE_T("MainSwapChain")
        });

        mIsDirectionalLightEnabled = true;
        mDirectionalLightDirection = Vector3(1.0f, 1.0f, 1.0f).Normalized();
        mDirectionalLightIntensity = Vector3(1.0f, 1.0f, 1.0f);

        mEnvironmentMapping.Initialize(true);
        mTonemap.Initialize();
        mRenderUtils.Initialize();

        mTextureViewer.Initialize(mNumGPUSync);

        LoadResources();
    }

    void Renderer::Shutdown(const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(Info, Renderer, "Shutdown renderer.");

        mGAPI->WaitAllGPUSync();

        ClearResources();

        mTextureViewer.Shutdown();

        mRenderUtils.Shutdown();
        mTonemap.Shutdown();
        mEnvironmentMapping.Shutdown();

        mCurrentBackbuffer = nullptr;
        mSwapChain = nullptr;

        mCommandList = nullptr;

        mPipelineManager.Shutdown();
        mSamplerManager.Shutdown();
        mTextureManager.Shutdown();
        mShaderManager.Shutdown();
        mShaderParameterListManager.Shutdown();

        mGAPI->Shutdown(imGUIContext);
        mGAPI = nullptr;
        mGAPI_DLib = nullptr;
    }

    void Renderer::OnLoopImGUIContent()
    {
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

        if (ImGui::CollapsingHeader("PostProcess", ImGuiTreeNodeFlags_DefaultOpen))
        {
            mTonemap.OnLoopImGUIContent();
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

            ImGui::Checkbox("Wireframe", &mWireframe);

            ImGui::SeparatorText("Texture Viewer");
            if (ImGui::Button("Show"))
            {
                mTextureViewer.Show();
            }
        }

        mTextureViewer.OnLoopImGUI();
    }

    void Renderer::RenderAndPresent()
    {
        mCurrentRenderingFrame++;
        mGAPI->BeginRenderingFrame();
        mShaderParameterListManager.MoveNextFrame();
        mTextureViewer.MoveToNextFrame();

        SetGlobalConstantBuffers();

        mSwapChain->AcquireNextImage();
        mCurrentBackbuffer = mSwapChain->GetCurrentBackbuffer();
        mCommandList->Reset();
        {
            mCommandList->Begin();
            
            mCommandList->ResourceTransition({
                .resourceType = gapi::TransitionState::ResourceType::Texture,
                .texture = mCurrentBackbuffer,
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
                .src =  gapi::ResourceStateFlag::Common,
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
        gapi::TimestampRangeList timestampRangeList = mGAPI->GetLastTimestampRangeList();

        const Vector<gapi::TimestampRange>& timestampRanges = timestampRangeList.timestampRanges;
        if (timestampRanges.size() >= 1)
        {
            const Uint64 gpuTime = (timestampRanges[0].endTime - timestampRanges[0].beginTime);
            return static_cast<float>(static_cast<double>(gpuTime) / static_cast<double>(timestampRangeList.frequency) * 1000.0);
        }
        else
        {
            return 0.0f;
        }
    }

    void Renderer::SetScene(SharedPtr<Scene> scene)
    {
        mScene = scene;
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
        gapi::RasterizerState::FillMode fillMode = mWireframe
            ? gapi::RasterizerState::FillMode::Line
            : gapi::RasterizerState::FillMode::Solid;

        RGBuilder builder(*this);

        {
            RG_GPU_EVENT_SCOPE(builder, CUBE_T("Frame"));

            gapi::TextureInfo colorTextureInfo = {
                .format = mColorFormat,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::RenderTarget,
                .width = mViewportWidth,
                .height = mViewportHeight
            };
            RGTextureHandle color = builder.CreateTexture(colorTextureInfo, CUBE_T("ColorBuffer"));
            gapi::TextureInfo depthTextureInfo = {
                .format = mDepthStencilFormat,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::DepthStencil,
                .width = mViewportWidth,
                .height = mViewportHeight
            };
            RGTextureHandle depthStencil = builder.CreateTexture(depthTextureInfo, CUBE_T("DepthStencilBuffer"));

            RGTextureRTVHandle colorRTV = builder.CreateRTV(color);
            RGTextureDSVHandle depthStencilDSV = builder.CreateDSV(depthStencil);
            {
                RG_GPU_TIMESTAMP_SCOPE(builder, CUBE_T("MainPass"));

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
                builder.BindShaderParameterList(globalShaderParameterList);

                auto envMapShaderParameterList = builder.CreateShaderParameterList<EnvironmentMapLightShaderParameterList>();
                envMapShaderParameterList->Get()->diffuseIrradianceMap = mEnvironmentMapping.GetDiffuseIrradianceMap(builder);
                envMapShaderParameterList->Get()->integratedBRDFLUT = mEnvironmentMapping.GetIntegratedBRDFLUT(builder);
                envMapShaderParameterList->Get()->prefilterMap = mEnvironmentMapping.GetPrefilterMap(builder);
                envMapShaderParameterList->Get()->prefilterSampler = mEnvironmentMapping.GetPrefilterMapSampler();
                envMapShaderParameterList->Get()->prefilterMapMipLevels = mEnvironmentMapping.GetPrefilterMapMipLevels();

                RGBuilder::RenderPassInfo renderPassInfo;
                renderPassInfo.colors.push_back({
                    .color = colorRTV,
                    .loadOperation = gapi::LoadOperation::Clear,
                    .storeOperation = gapi::StoreOperation::Store,
                    .clearColor = { 0.2f, 0.2f, 0.2f, 1.0f }
                });
                renderPassInfo.depthStencil = {
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

                const gapi::RasterizerState mainPassRasterizerState = {
                    .fillMode = fillMode
                };
                const gapi::DepthStencilState mainPassDepthStencilState = {
                    .enableDepth = true,
                    .depthFunction = gapi::CompareFunction::Greater
                };

                if (mScene)
                {
                    FrameVector<RGBuilder::DrawMeshInfo> drawMeshInfos;
                    for (const UniquePtr<SceneObject>& sceneObject : mScene->GetSceneObjects())
                    {
                        if (SharedPtr<Mesh> mesh = sceneObject->GetMesh())
                        {
                            drawMeshInfos.push_back({
                                .mesh = mesh,
                                .rasterizerState = mainPassRasterizerState,
                                .depthStencilState = mainPassDepthStencilState,
                                .materials = sceneObject->GetMaterials(),
                                .model = sceneObject->GetModelMatrix() * mModelMatrix
                            });
                        }
                    }

                    builder.AddDrawMeshPass(CUBE_T("Draw Scene"), drawMeshInfos, RGBuilder::MakeParameterListArray(envMapShaderParameterList));
                }

                if (mShowAxis)
                {
                    FrameVector<RGBuilder::DrawMeshInfo> drawAxisMeshInfos;
                    WeakPtr<Material> xAxisMaterial = mXAxisMaterial;
                    drawAxisMeshInfos.push_back({
                        .mesh = mBoxMesh,
                        .rasterizerState = mainPassRasterizerState,
                        .depthStencilState = mainPassDepthStencilState,
                        .materials = { &xAxisMaterial, 1 },
                        .model = mXAxisModelMatrix,
                    });
                    WeakPtr<Material> yAxisMaterial = mYAxisMaterial;
                    drawAxisMeshInfos.push_back({
                        .mesh = mBoxMesh,
                        .rasterizerState = mainPassRasterizerState,
                        .depthStencilState = mainPassDepthStencilState,
                        .materials = { &yAxisMaterial, 1 },
                        .model = mYAxisModelMatrix
                    });
                    WeakPtr<Material> zAxisMaterial = mZAxisMaterial;
                    drawAxisMeshInfos.push_back({
                        .mesh = mBoxMesh,
                        .rasterizerState = mainPassRasterizerState,
                        .depthStencilState = mainPassDepthStencilState,
                        .materials = { &zAxisMaterial, 1 },
                        .model = mZAxisModelMatrix
                    });
                    builder.AddDrawMeshPass(CUBE_T("Draw Axis"), drawAxisMeshInfos, RGBuilder::MakeParameterListArray(envMapShaderParameterList));
                }

                mEnvironmentMapping.DrawSkybox(builder);

                builder.EndRenderPass();
            }

            const gapi::TextureInfo tonemappedColorTextureInfo = {
                .format = mColorFormat,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::UAV,
                .width = color->GetTextureInfo().width,
                .height = color->GetTextureInfo().height
            };
            RGTextureHandle tonemappedColor = builder.CreateTexture(tonemappedColorTextureInfo, CUBE_T("Tonemapped Color"));
            mTonemap.Execute(builder, color, tonemappedColor);

            RGTextureHandle backbuffer = builder.RegisterTexture(mCurrentBackbuffer);
            mRenderUtils.CopyTexturePS(builder, tonemappedColor, backbuffer);

            {
                RG_GPU_TIMESTAMP_SCOPE(builder, CUBE_T("Texture Viewer"));

                mTextureViewer.Update(builder);
            }
        }
        builder.ExecuteAndSubmit(*mCommandList);
    }

    void Renderer::LoadResources()
    {
        // Use default mesh metadata.
        mBoxMesh = std::make_shared<Mesh>(MeshHelper::GenerateBoxMeshData(), MeshMetadata{});
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
    }

    void Renderer::ClearResources()
    {
        mEnvironmentMapping.ClearResources();

        mZAxisMaterial = nullptr;
        mYAxisMaterial = nullptr;
        mXAxisMaterial = nullptr;

        mDummyWhiteTexture2D = nullptr;
        mDummyBlackTextureCube = nullptr;
        mDummyBlackTexture2D = nullptr;
        mDefaultMaterial = nullptr;

        mScene = nullptr;
        mBoxMesh = nullptr;
    }
} // namespace cube
