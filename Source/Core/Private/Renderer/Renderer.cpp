#include "Renderer.h"

#include "imguizmo_quat/imGuIZMOquat.h"
#include "imgui.h"

#include "BaseMeshGenerator.h"
#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI_Buffer.h"
#include "GAPI_CommandList.h"
#include "GAPI_Shader.h"
#include "GAPI_ShaderVariable.h"
#include "GAPI_Pipeline.h"
#include "GAPI_Viewport.h"
#include "Material.h"
#include "MatrixUtility.h"
#include "Platform.h"
#include "Texture.h"

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

        mGAPI->Initialize({
            .numGPUSync = numGPUSync,
            .enableDebugLayer = true,
            .imGUI = imGUIContext
        });

        mTextureManager.Initialize(mGAPI.get(), mNumGPUSync);
        mSamplerManager.Initialize(mGAPI.get());
        mShaderParametersManager.Initialize(mGAPI.get(), mNumGPUSync);

        mRenderImGUI = (imGUIContext.context != nullptr);

        mIsViewPerspectiveMatrixDirty = true;

        mCommandList = mGAPI->CreateCommandList({
            .debugName = CUBE_T("MainCommandList")
        });

        mViewportWidth = platform::Platform::GetWindowWidth();
        mViewportHeight = platform::Platform::GetWindowHeight();
        mViewport = mGAPI->CreateViewport({
            .width = mViewportWidth,
            .height = mViewportHeight,
            .vsync = true,
            .backbufferCount = 2,
            .debugName = CUBE_T("MainViewport")
        });

        mDirectionalLightDirection = Vector3(1.0f, 1.0f, 1.0f).Normalized();
        mIsDirectionalLightDirty = true;

        LoadResources();
    }

    void Renderer::Shutdown(const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(Info, Renderer, "Shutdown renderer.");

        mGAPI->WaitAllGPUSync();

        ClearResources();

        mViewport = nullptr;

        mCommandList = nullptr;

        mShaderParametersManager.Shutdown();
        mSamplerManager.Shutdown();
        mTextureManager.Shutdown();

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

                imguiGizmo::resizeAxesOf({ 0.7f, 0.8f, 0.8f });
                ImGui::gizmo3D("##Directional Light - Direction", directionVec3);
                imguiGizmo::restoreAxesSize();

                mDirectionalLightDirection = Vector3(directionVec3.x, directionVec3.y, directionVec3.z);
                mIsDirectionalLightDirty = true;
            }
        }

        ImGui::End();
    }

    void Renderer::Render()
    {
        mCurrentRenderingFrame++;
        mGAPI->BeginRenderingFrame();
        mShaderParametersManager.MoveNextFrame();

        SetGlobalConstantBuffers();

        mViewport->AcquireNextImage();

        mGAPI->OnBeforeRender();

        RenderImpl();

        mGAPI->OnAfterRender();

        if (mRenderImGUI)
        {
            ImGui::Render();
        }

        mGAPI->OnBeforePresent(mViewport.get());
        mViewport->Present();
        mGAPI->OnAfterPresent();

        mGAPI->EndRenderingFrame();
    }

    void Renderer::OnResize(Uint32 width, Uint32 height)
    {
        mViewportWidth = width;
        mViewportHeight = height;

        if (mViewport)
        {
            mGAPI->WaitAllGPUSync();

            mViewport->Resize(width, height);
        }
    }

    void Renderer::SetObjectModelMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale)
    {
        mModelMatrix = MatrixUtility::GetScale(scale) * MatrixUtility::GetRotationXYZ(rotation) + MatrixUtility::GetTranslation(position);
    }

    void Renderer::SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& upDir)
    {
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
        if (mGAPI->GetInfo().useLeftHanded) {
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
            mMesh = std::make_shared<Mesh>(BaseMeshGenerator::GetBoxMeshData());
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
        mCommandList->Reset();
        {
            mCommandList->Begin();

            mCommandList->InsertTimestamp(CUBE_T("Begin"));

            mCommandList->SetViewports({ &mViewport, 1 });
            gapi::ScissorRect scissor = {
                .x = 0,
                .y = 0,
                .width = mViewportWidth,
                .height = mViewportHeight
            };
            mCommandList->SetScissors({ &scissor, 1 });
            mCommandList->SetPrimitiveTopology(gapi::PrimitiveTopology::TriangleList);

            mCommandList->ResourceTransition({
                .resourceType =  gapi::TransitionState::ResourceType::ViewportBackBuffer,
                .viewport = mViewport,
                .src = gapi::ResourceStateFlag::Present,
                .dst = gapi::ResourceStateFlag::RenderTarget
            });

            mCommandList->SetShaderVariablesLayout(mShaderVariablesLayout);
            mCommandList->SetRenderTarget(mViewport);
            mCommandList->ClearRenderTargetView(mViewport, { 0.2f, 0.2f, 0.2f, 1.0f });
            mCommandList->ClearDepthStencilView(mViewport, 0);
            mCommandList->SetGraphicsPipeline(mMainPipeline);

            SharedPtr<GlobalShaderParameters> globalShaderParameters = mShaderParametersManager.CreateShaderParameters<GlobalShaderParameters>();
            globalShaderParameters->viewProjection = mViewPerspectiveMatirx;
            globalShaderParameters->directionalLightDirection = mDirectionalLightDirection;
            globalShaderParameters->WriteAllParametersToBuffer();
            mCommandList->SetShaderVariableConstantBuffer(0, globalShaderParameters->GetBuffer());

            Uint32 vertexBufferOffset = 0;
            {
                // Center object
                SharedPtr<gapi::Buffer> vertexBuffer = mMesh->GetVertexBuffer();
                mCommandList->BindVertexBuffers(0, { &vertexBuffer, 1 }, { &vertexBufferOffset, 1 });
                mCommandList->BindIndexBuffer(mMesh->GetIndexBuffer(), 0);

                SharedPtr<ObjectShaderParameters> objectShaderParameters = mShaderParametersManager.CreateShaderParameters<ObjectShaderParameters>();
                objectShaderParameters->model = mModelMatrix;
                objectShaderParameters->modelInverse = mModelMatrix.Inversed();
                objectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, objectShaderParameters->GetBuffer());

                int currentMaterialIndex = -2;
                const Vector<SubMesh>& subMeshes = mMesh->GetSubMeshes();
                for (const SubMesh& subMesh : subMeshes)
                {
                    int lastMaterialIndex = currentMaterialIndex;
                    if (subMesh.materialIndex < mMaterials.size())
                    {
                        currentMaterialIndex = subMesh.materialIndex;
                    }
                    else
                    {
                        // Use default material
                        currentMaterialIndex = -1;
                    }

                    if (currentMaterialIndex != lastMaterialIndex)
                    {
                        if (currentMaterialIndex != -1)
                        {
                            SharedPtr<MaterialShaderParameters> materialShaderParameters = mMaterials[currentMaterialIndex]->GenerateShaderParameters();
                            mCommandList->SetShaderVariableConstantBuffer(2, materialShaderParameters->GetBuffer());
                        }
                        else
                        {
                            SharedPtr<MaterialShaderParameters> materialShaderParameters = mDefaultMaterial->GenerateShaderParameters();
                            mCommandList->SetShaderVariableConstantBuffer(2, materialShaderParameters->GetBuffer());
                        }
                    }

                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
            }

            {
                // Axis
                SharedPtr<gapi::Buffer> boxVertexBuffer = mBoxMesh->GetVertexBuffer();
                mCommandList->BindVertexBuffers(0, { &boxVertexBuffer, 1 }, { &vertexBufferOffset, 1 });
                mCommandList->BindIndexBuffer(mBoxMesh->GetIndexBuffer(), 0);
                const Vector<SubMesh>& boxSubMeshes = mBoxMesh->GetSubMeshes();

                SharedPtr<ObjectShaderParameters> xAxisObjectShaderParameters = mShaderParametersManager.CreateShaderParameters<ObjectShaderParameters>();
                xAxisObjectShaderParameters->model = mXAxisModelMatrix;
                xAxisObjectShaderParameters->modelInverse = mXAxisModelMatrix.Inversed();
                xAxisObjectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, xAxisObjectShaderParameters->GetBuffer());
                SharedPtr<MaterialShaderParameters> xAxisMaterialShaderParameters = mXAxisMaterial->GenerateShaderParameters();
                mCommandList->SetShaderVariableConstantBuffer(2, xAxisMaterialShaderParameters->GetBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                SharedPtr<ObjectShaderParameters> yAxisObjectShaderParameters = mShaderParametersManager.CreateShaderParameters<ObjectShaderParameters>();
                yAxisObjectShaderParameters->model = mYAxisModelMatrix;
                yAxisObjectShaderParameters->modelInverse = mYAxisModelMatrix.Inversed();
                yAxisObjectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, yAxisObjectShaderParameters->GetBuffer());
                SharedPtr<MaterialShaderParameters> yAxisMaterialShaderParameters = mYAxisMaterial->GenerateShaderParameters();
                mCommandList->SetShaderVariableConstantBuffer(2, yAxisMaterialShaderParameters->GetBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                SharedPtr<ObjectShaderParameters> zAxisObjectShaderParameters = mShaderParametersManager.CreateShaderParameters<ObjectShaderParameters>();
                zAxisObjectShaderParameters->model = mZAxisModelMatrix;
                zAxisObjectShaderParameters->modelInverse = mZAxisModelMatrix.Inversed();
                zAxisObjectShaderParameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(1, zAxisObjectShaderParameters->GetBuffer());
                SharedPtr<MaterialShaderParameters> zAxisMaterialShaderParameters = mZAxisMaterial->GenerateShaderParameters();
                mCommandList->SetShaderVariableConstantBuffer(2, zAxisMaterialShaderParameters->GetBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
            }

            mCommandList->ResourceTransition({
                .resourceType =  gapi::TransitionState::ResourceType::ViewportBackBuffer,
                .viewport = mViewport,
                .src = gapi::ResourceStateFlag::RenderTarget,
                .dst = gapi::ResourceStateFlag::Present
            });

            mCommandList->InsertTimestamp(CUBE_T("End"));

            mCommandList->End();
        }
        mCommandList->Submit();
    }

    void Renderer::LoadResources()
    {
        mBoxMesh = std::make_shared<Mesh>(BaseMeshGenerator::GetBoxMeshData());
        SetMesh(nullptr); // Load default mesh

        {
            FrameString vertexShaderFilePath = FrameString(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/Main.slang");
            SharedPtr<platform::File> vertexShaderFile = platform::FileSystem::OpenFile(vertexShaderFilePath, platform::FileAccessModeFlag::Read);
            CHECK(vertexShaderFile);
            Uint64 vertexShaderFileSize = vertexShaderFile->GetFileSize();

            Blob shaderCode(vertexShaderFileSize);
            Uint64 readSize = vertexShaderFile->Read(shaderCode.GetData(), vertexShaderFileSize);
            CHECK(readSize <= vertexShaderFileSize);

            mVertexShader = mGAPI->CreateShader({
                .type = gapi::ShaderType::Vertex,
                .language = gapi::ShaderLanguage::Slang,
                .fileName = StringView(CUBE_T("Main.slang")),
                .path = vertexShaderFilePath,
                .code = shaderCode,
                .entryPoint = "VSMain",
                .withDebugSymbol = true, // TODO: Add option in render ui after implement shader recompilation
                .debugName = CUBE_T("MainVS")
            });
            CHECK(mVertexShader);
        }
        {
            FrameString pixelShaderFilePath = FrameString(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/Main.slang");
            SharedPtr<platform::File> pixelShaderFile = platform::FileSystem::OpenFile(pixelShaderFilePath, platform::FileAccessModeFlag::Read);
            CHECK(pixelShaderFile);
            Uint64 pixelShaderFileSize = pixelShaderFile->GetFileSize();

            Blob shaderCode(pixelShaderFileSize);
            Uint64 readSize = pixelShaderFile->Read(shaderCode.GetData(), pixelShaderFileSize);
            CHECK(readSize <= pixelShaderFileSize);

            mPixelShader = mGAPI->CreateShader({
                .type = gapi::ShaderType::Pixel,
                .language = gapi::ShaderLanguage::Slang,
                .fileName = StringView(CUBE_T("Main.slang")),
                .path = pixelShaderFilePath,
                .code = shaderCode,
                .entryPoint = "PSMain",
                .withDebugSymbol = true, // TODO: Add option in render ui after implement shader recompilation
                .debugName = CUBE_T("MainPS")
            });
            CHECK(mPixelShader);
        }
        {
            mShaderVariablesLayout = mGAPI->CreateShaderVariablesLayout({
                .numShaderVariablesConstantBuffer = 3,
                .shaderVariablesConstantBuffer = nullptr,
                .debugName = CUBE_T("MainShaderVariablesLayout")
            });
        }
        {
            constexpr int positionOffset = 0;
            constexpr int colorOffset = positionOffset + sizeof(Vertex::position);
            constexpr int normalOffset = colorOffset + sizeof(Vertex::color);
            constexpr int uvOffset = normalOffset + sizeof(Vertex::normal); 

            gapi::InputElement inputLayout[] = {
                {
                    .name = "POSITION",
                    .index = 0,
                    .format = gapi::ElementFormat::RGB32_Float,
                    .offset = positionOffset,
                },
                {
                    .name = "COLOR",
                    .index = 0,
                    .format = gapi::ElementFormat::RGBA32_Float,
                    .offset = colorOffset,
                },
                {
                    .name = "NORMAL",
                    .index = 0,
                    .format = gapi::ElementFormat::RGB32_Float,
                    .offset = normalOffset
                },
                {
                    .name = "TEXCOORD",
                    .index = 0,
                    .format = gapi::ElementFormat::RG32_Float,
                    .offset = uvOffset
                }
            };

            mMainPipeline = mGAPI->CreateGraphicsPipeline({
                .vertexShader = mVertexShader,
                .pixelShader = mPixelShader,
                .inputLayout = inputLayout,
                .numInputLayoutElements = std::size(inputLayout),
                .depthStencilState = {
                    .enableDepth = true,
                    .depthFunction = gapi::CompareFunction::Greater
                },
                .numRenderTargets = 1,
                .renderTargetFormats = { gapi::ElementFormat::RGBA8_UNorm },
                .shaderVariablesLayout = mShaderVariablesLayout,
                .debugName = CUBE_T("MainPipeline")
            });
        }

        mDefaultMaterial = std::make_shared<Material>(CUBE_T("DefaultMaterial"));
        mDefaultMaterial->SetBaseColor({ 1.0f, 0.0f, 0.80392f }); // Magenta

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

        mMainPipeline = nullptr;
        mShaderVariablesLayout = nullptr;
        mPixelShader = nullptr;
        mVertexShader = nullptr;

        mMaterials.clear();
        mMesh = nullptr;
        mBoxMesh = nullptr;
    }
} // namespace cube
