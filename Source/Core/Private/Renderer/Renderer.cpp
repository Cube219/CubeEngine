#include "Renderer.h"

#include "imguizmo_quat/imGuIZMOquat.h"
#include "imgui.h"

#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI_Buffer.h"
#include "GAPI_CommandList.h"
#include "GAPI_Pipeline.h"
#include "GAPI_Shader.h"
#include "GAPI_SwapChain.h"
#include "Material.h"
#include "MatrixUtility.h"
#include "MeshHelper.h"
#include "Platform.h"
#include "RenderProfiling.h"
#include "Shader.h"

namespace cube
{
    void Renderer::Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext, Uint32 numGPUSync)
    {
        CUBE_LOG(Info, Renderer, "Initialize renderer. (GAPI: {})", GAPINameToString(gAPIName));

        mNumGPUSync = numGPUSync;
        mCurrentRenderingFrame = 0;

        // GAPI init
        const Character* dLibName = CUBE_T("");
        switch (gAPIName)
        {
        case GAPIName::DX12:
            dLibName = CUBE_T("CE-GAPI_DX12");
            break;
        case GAPIName::Metal:
            dLibName = CUBE_T("CE-GAPI_Metal");
            break;

        case GAPIName::Unknown:
        default:
            CHECK_FORMAT(false, "Invalid GAPIName: {}", (int)gAPIName);
            break;
        }

        mGAPI_DLib = platform::Platform::LoadDLib(dLibName);
        CHECK_FORMAT(mGAPI_DLib, "Cannot load GAPI library! (Name: {})", dLibName);

        using CreateGAPIFunction = GAPI* (*)();
        auto createGAPIFunc = reinterpret_cast<CreateGAPIFunction>(mGAPI_DLib->GetFunction(CUBE_T("CreateGAPI")));
        mGAPI = SharedPtr<GAPI>(createGAPIFunc());

        mGAPI->Initialize({ .numGPUSync = numGPUSync,
                            .enableDebugLayer = true,
                            .imGUI = imGUIContext });

        mShaderManager.Initialize(mGAPI.get(), false);
        mTextureManager.Initialize(mGAPI.get(), mNumGPUSync, mShaderManager);
        mSamplerManager.Initialize(mGAPI.get());
        mShaderParametersManager.Initialize(mGAPI.get(), mNumGPUSync);

        mRenderImGUI = (imGUIContext.context != nullptr);

        mIsViewPerspectiveMatrixDirty = true;

        mCommandList = mGAPI->CreateCommandList({ .debugName = CUBE_T("MainCommandList") });

        mViewportWidth = platform::Platform::GetWindowWidth();
        mViewportHeight = platform::Platform::GetWindowHeight();
        mSwapChain = mGAPI->CreateSwapChain({ .width = mViewportWidth,
                                              .height = mViewportHeight,
                                              .vsync = true,
                                              .backbufferCount = 2,
                                              .debugName = CUBE_T("MainSwapChain") });
        mDepthStencilTexture = mGAPI->CreateTexture({ .usage = gapi::ResourceUsage::GPUOnly,
                                                      .format = gapi::ElementFormat::D32_Float,
                                                      .type = gapi::TextureType::Texture2D,
                                                      .flags = gapi::TextureFlag::DepthStencil,
                                                      .width = mViewportWidth,
                                                      .height = mViewportHeight,
                                                      .debugName = CUBE_T("MainDepthStencilTexture") });
        mDSV = mDepthStencilTexture->CreateDSV({});
        mCommandList->Reset();
        {
            mCommandList->Begin();

            mCommandList->ResourceTransition({ .resourceType = gapi::TransitionState::ResourceType::DSV,
                                               .dsv = mDSV,
                                               .src = gapi::ResourceStateFlag::Common,
                                               .dst = gapi::ResourceStateFlag::DepthWrite });

            mCommandList->End();
            mCommandList->Submit();
        }

        mDirectionalLightDirection = Vector3(1.0f, 1.0f, 1.0f).Normalized();
        mIsDirectionalLightDirty = true;

        LoadResources();
    }

    void Renderer::Shutdown(const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(Info, Renderer, "Shutdown renderer.");

        mGAPI->WaitAllGPUSync();

        ClearResources();

        mDSV = nullptr;
        mDepthStencilTexture = nullptr;
        mCurrentBackbufferRTV = nullptr;
        mSwapChain = nullptr;

        mCommandList = nullptr;

        mShaderParametersManager.Shutdown();
        mSamplerManager.Shutdown();
        mTextureManager.Shutdown();
        mShaderManager.Shutdown();

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
                mIsDirectionalLightDirty = true;
            }
        }

        ImGui::SeparatorText("Shader");

        if (ImGui::Checkbox("Shader Debug Mode", &mShaderManager.mUseDebugMode))
        {
            mShaderManager.RecompileShaders(true);
        }

        if (ImGui::Button("RecompileShader"))
        {
            mShaderManager.RecompileShaders();
        }

        ImGui::End();
    }

    void Renderer::RenderAndPresent()
    {
        mCurrentRenderingFrame++;
        mGAPI->BeginRenderingFrame();
        mShaderParametersManager.MoveNextFrame();

        SetGlobalConstantBuffers();

        mSwapChain->AcquireNextImage();
        mCurrentBackbufferRTV = mSwapChain->GetCurrentBackbufferRTV();
        mCommandList->Reset();
        {
            mCommandList->Begin();

            mCommandList->ResourceTransition({ .resourceType = gapi::TransitionState::ResourceType::RTV,
                                               .rtv = mCurrentBackbufferRTV,
                                               .src = gapi::ResourceStateFlag::Present,
                                               .dst = gapi::ResourceStateFlag::RenderTarget });

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

        mGAPI->OnBeforePresent(mCurrentBackbufferRTV.get());
        mCommandList->Reset();
        {
            mCommandList->Begin();

            mCommandList->ResourceTransition({ .resourceType = gapi::TransitionState::ResourceType::RTV,
                                               .rtv = mCurrentBackbufferRTV,
                                               .src = gapi::ResourceStateFlag::RenderTarget,
                                               .dst = gapi::ResourceStateFlag::Present });

            mCommandList->End();
            mCommandList->Submit();
        }
        mSwapChain->Present();
        mGAPI->OnAfterPresent();

        mCurrentBackbufferRTV = nullptr;

        mGAPI->EndRenderingFrame();
    }

    void Renderer::OnResize(Uint32 width, Uint32 height)
    {
        mViewportWidth = width;
        mViewportHeight = height;

        mGAPI->WaitAllGPUSync();

        if (mSwapChain)
        {
            mCurrentBackbufferRTV = nullptr;
            mSwapChain->Resize(width, height);
        }

        if (mDepthStencilTexture)
        {
            mDSV = nullptr;
            mDepthStencilTexture = nullptr;

            mDepthStencilTexture = mGAPI->CreateTexture({
                .usage = gapi::ResourceUsage::GPUOnly,
                .format = gapi::ElementFormat::D32_Float,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::DepthStencil,
                .width = mViewportWidth,
                .height = mViewportHeight,
                .debugName = CUBE_T("MainDepthStencilTexture")
            });
            mDSV = mDepthStencilTexture->CreateDSV({});

            mCommandList->Reset();
            mCommandList->Begin();

            mCommandList->ResourceTransition({
                .resourceType = gapi::TransitionState::ResourceType::DSV,
                .dsv = mDSV,
                .src = gapi::ResourceStateFlag::Common,
                .dst = gapi::ResourceStateFlag::DepthWrite
            });

            mCommandList->End();
            mCommandList->Submit();
        }
    }

    void Renderer::SetObjectModelMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale)
    {
        mModelMatrix = MatrixUtility::GetScale(scale) * MatrixUtility::GetRotationXYZ(rotation) + MatrixUtility::GetTranslation(position);
    }

    void Renderer::SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& upDir)
    {
        mViewPosition = eye;
        if (mGAPI->GetInfo().useLeftHanded)
        {
            // Flip z axis
            mViewMatrix = MatrixUtility::GetLookAt(target, eye, upDir);
        }
        else
        {
            mViewMatrix = MatrixUtility::GetLookAt(eye, target, upDir);
        }
        mIsViewPerspectiveMatrixDirty = true;
    }

    void Renderer::SetPerspectiveMatrix(float fovAngleY, float aspectRatio, float nearZ, float farZ)
    {
        mPerspectiveMatrix = MatrixUtility::GetPerspectiveFov(fovAngleY, aspectRatio, nearZ, farZ);
        if (mGAPI->GetInfo().useLeftHanded)
        {
            // Flip z axis
            Vector4 zRow = mPerspectiveMatrix.GetRow(2);
            mPerspectiveMatrix.SetRow(2, -zRow);
        }
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

    void Renderer::SetMesh(SharedPtr<MeshData> meshData)
    {
        if (meshData)
        {
            mMesh = std::make_shared<Mesh>(meshData);
        }
        else
        {
            mMesh = std::make_shared<Mesh>(MeshHelper::GenerateBoxMeshData());
        }
    }

    void Renderer::SetMaterials(const Vector<SharedPtr<Material>>& materials)
    {
        mMaterials = materials;
    }

    void Renderer::SetGlobalConstantBuffers()
    {
        bool updateNeeded = false;

        if (mIsViewPerspectiveMatrixDirty)
        {
            mViewPerspectiveMatirx = mViewMatrix * mPerspectiveMatrix;
            mIsViewPerspectiveMatrixDirty = false;
        }
    }

    void Renderer::RenderImpl()
    {
        GraphicsPipeline* currentGraphicsPipeline = nullptr;
        auto SetGraphicsPipeline = [this, &currentGraphicsPipeline](SharedPtr<GraphicsPipeline> graphicsPipeline)
        {
            if (currentGraphicsPipeline != graphicsPipeline.get())
            {
                mCommandList->SetGraphicsPipeline(graphicsPipeline->GetGAPIGraphicsPipeline());
                currentGraphicsPipeline = graphicsPipeline.get();
            }
        };

        mCommandList->Reset();
        mCommandList->Begin();
        {

            mCommandList->InsertTimestamp(CUBE_T("Begin"));

            GPU_EVENT_SCOPE(mCommandList, CUBE_T("Frame"));

            gapi::Viewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(mViewportWidth),
                .height = static_cast<float>(mViewportHeight)
            };
            mCommandList->SetViewports({ &viewport, 1 });
            gapi::ScissorRect scissor = {
                .x = 0,
                .y = 0,
                .width = mViewportWidth,
                .height = mViewportHeight
            };
            mCommandList->SetScissors({ &scissor, 1 });
            mCommandList->SetPrimitiveTopology(gapi::PrimitiveTopology::TriangleList);

            gapi::ColorAttachment colorAttachment = {
                .rtv = mCurrentBackbufferRTV,
                .loadOperation = gapi::LoadOperation::Clear,
                .storeOperation = gapi::StoreOperation::Store,
                .clearColor = { 0.2f, 0.2f, 0.2f, 1.0f }
            };
            gapi::DepthStencilAttachment depthStencilAttachment = {
                .dsv = mDSV,
                .loadOperation = gapi::LoadOperation::Clear,
                .storeOperation = gapi::StoreOperation::Store,
                .clearDepth = 0.0f
            };
            mCommandList->SetRenderTargets({ &colorAttachment, 1 }, depthStencilAttachment);

            SharedPtr<GlobalShaderParameters> globalShaderParameters = mShaderParametersManager.CreateShaderParameters<GlobalShaderParameters>();
            globalShaderParameters->viewPosition = mViewPosition;
            globalShaderParameters->viewProjection = mViewPerspectiveMatirx;
            globalShaderParameters->directionalLightDirection = mDirectionalLightDirection;
            globalShaderParameters->WriteAllParametersToBuffer();
            mCommandList->SetShaderVariableConstantBuffer(0, globalShaderParameters->GetBuffer());

            Uint32 vertexBufferOffset = 0;
            {
                GPU_EVENT_SCOPE(mCommandList, CUBE_T("Draw Center Object"));

                // Center object
                SharedPtr<gapi::Buffer> vertexBuffer = mMesh->GetVertexBuffer();
                mCommandList->BindVertexBuffers(0, { &vertexBuffer, 1 }, { &vertexBufferOffset, 1 });
                mCommandList->BindIndexBuffer(mMesh->GetIndexBuffer(), 0);

                SharedPtr<PerObjectShaderParameters> perObjectShaderParameters = mShaderParametersManager.CreateShaderParameters<PerObjectShaderParameters>();
                perObjectShaderParameters->model = mModelMatrix;
                perObjectShaderParameters->modelInverse = mModelMatrix.Inversed();
                perObjectShaderParameters->modelInverseTranspose = mModelMatrix.Inversed().Transposed();
                perObjectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, perObjectShaderParameters->GetBuffer());

                SharedPtr<Material> currentMaterial = nullptr;
                const Vector<SubMesh>& subMeshes = mMesh->GetSubMeshes();
                for (const SubMesh& subMesh : subMeshes)
                {
                    SharedPtr<Material> lastMaterial = currentMaterial;
                    if (subMesh.materialIndex < mMaterials.size())
                    {
                        currentMaterial = mMaterials[subMesh.materialIndex];
                    }
                    else
                    {
                        currentMaterial = mDefaultMaterial;
                    }

                    GPU_EVENT_SCOPE(mCommandList, Format<FrameString>(CUBE_T("Mesh: {0}, Material: {1}"), subMesh.debugName, currentMaterial->GetDebugName()));

                    if (currentMaterial != lastMaterial)
                    {
                        SetGraphicsPipeline(mShaderManager.GetMaterialShaderManager().GetOrCreateMaterialPipeline(currentMaterial));
                        SharedPtr<MaterialShaderParameters> materialShaderParameters = currentMaterial->GenerateShaderParameters();
                        mCommandList->SetShaderVariableConstantBuffer(2, materialShaderParameters->GetBuffer());
                    }

                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
            }

            {
                GPU_EVENT_SCOPE(mCommandList, CUBE_T("Draw Axis"));
                // Axis
                SetGraphicsPipeline(mShaderManager.GetMaterialShaderManager().GetOrCreateMaterialPipeline(mDefaultMaterial));

                SharedPtr<gapi::Buffer> boxVertexBuffer = mBoxMesh->GetVertexBuffer();
                mCommandList->BindVertexBuffers(0, { &boxVertexBuffer, 1 }, { &vertexBufferOffset, 1 });
                mCommandList->BindIndexBuffer(mBoxMesh->GetIndexBuffer(), 0);
                const Vector<SubMesh>& boxSubMeshes = mBoxMesh->GetSubMeshes();

                SharedPtr<PerObjectShaderParameters> xAxisObjectShaderParameters = mShaderParametersManager.CreateShaderParameters<PerObjectShaderParameters>();
                xAxisObjectShaderParameters->model = mXAxisModelMatrix;
                xAxisObjectShaderParameters->modelInverse = mXAxisModelMatrix.Inversed();
                xAxisObjectShaderParameters->modelInverseTranspose = mXAxisModelMatrix.Inversed().Transposed();
                xAxisObjectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, xAxisObjectShaderParameters->GetBuffer());
                SharedPtr<MaterialShaderParameters> xAxisMaterialShaderParameters = mXAxisMaterial->GenerateShaderParameters();
                mCommandList->SetShaderVariableConstantBuffer(2, xAxisMaterialShaderParameters->GetBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                SharedPtr<PerObjectShaderParameters> yAxisObjectShaderParameters = mShaderParametersManager.CreateShaderParameters<PerObjectShaderParameters>();
                yAxisObjectShaderParameters->model = mYAxisModelMatrix;
                yAxisObjectShaderParameters->modelInverse = mYAxisModelMatrix.Inversed();
                yAxisObjectShaderParameters->modelInverseTranspose = mYAxisModelMatrix.Inversed().Transposed();
                yAxisObjectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, yAxisObjectShaderParameters->GetBuffer());
                SharedPtr<MaterialShaderParameters> yAxisMaterialShaderParameters = mYAxisMaterial->GenerateShaderParameters();
                mCommandList->SetShaderVariableConstantBuffer(2, yAxisMaterialShaderParameters->GetBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                SharedPtr<PerObjectShaderParameters> zAxisObjectShaderParameters = mShaderParametersManager.CreateShaderParameters<PerObjectShaderParameters>();
                zAxisObjectShaderParameters->model = mZAxisModelMatrix;
                zAxisObjectShaderParameters->modelInverse = mZAxisModelMatrix.Inversed();
                zAxisObjectShaderParameters->modelInverseTranspose = mZAxisModelMatrix.Inversed().Transposed();
                zAxisObjectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, zAxisObjectShaderParameters->GetBuffer());
                SharedPtr<MaterialShaderParameters> zAxisMaterialShaderParameters = mZAxisMaterial->GenerateShaderParameters();
                mCommandList->SetShaderVariableConstantBuffer(2, zAxisMaterialShaderParameters->GetBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
            }

            mCommandList->InsertTimestamp(CUBE_T("End"));
        }
        mCommandList->End();
        mCommandList->Submit();
    }

    void Renderer::LoadResources()
    {
        mBoxMesh = std::make_shared<Mesh>(MeshHelper::GenerateBoxMeshData());
        SetMesh(nullptr); // Load default mesh

        mDefaultMaterial = std::make_shared<Material>(CUBE_T("DefaultMaterial"));

        {
            mXAxisModelMatrix = MatrixUtility::GetScale(4.0f, 0.2f, 0.2f) + MatrixUtility::GetTranslation(2, 0, 0);
            mXAxisMaterial = std::make_shared<Material>(CUBE_T("XAxisMaterial"));
            mXAxisMaterial->SetBaseColor({ 1.0f, 0.0f, 0.0f, 1.0f });

            mYAxisModelMatrix = MatrixUtility::GetScale(0.2f, 4.0f, 0.2f) + MatrixUtility::GetTranslation(0, 2, 0);
            mYAxisMaterial = std::make_shared<Material>(CUBE_T("YAxisMaterial"));
            mYAxisMaterial->SetBaseColor({ 0.0f, 1.0f, 0.0f, 1.0f });

            mZAxisModelMatrix = MatrixUtility::GetScale(0.2f, 0.2f, 4.0f) + MatrixUtility::GetTranslation(0, 0, 2);
            mZAxisMaterial = std::make_shared<Material>(CUBE_T("ZAxisMaterial"));
            mZAxisMaterial->SetBaseColor({ 0.0f, 0.0f, 1.0f, 1.0f });
        }
    }

    void Renderer::ClearResources()
    {
        mZAxisMaterial = nullptr;
        mYAxisMaterial = nullptr;
        mXAxisMaterial = nullptr;

        mDefaultMaterial = nullptr;

        mMaterials.clear();
        mMesh = nullptr;
        mBoxMesh = nullptr;
    }
} // namespace cube
