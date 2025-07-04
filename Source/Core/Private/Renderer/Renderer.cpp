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
    void Renderer::Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(Info, Renderer, "Initialize renderer. (GAPI: {})", GAPINameToString(gAPIName));

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
            .enableDebugLayer = true,
            .imGUI = imGUIContext
        });

        mTextureManager.Initialize(mGAPI.get());
        mSamplerManager.Initialize(mGAPI.get());

        mRenderImGUI = (imGUIContext.context != nullptr);

        mIsViewPerspectiveMatrixDirty = true;
        mGlobalConstantBuffer = mGAPI->CreateBuffer({
            .type = gapi::BufferType::Constant,
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = sizeof(GlobalConstantBufferData),
            .debugName = CUBE_T("GlobalConstantBuffer")
        });
        mGlobalConstantBufferDataPointer = static_cast<Uint8*>(mGlobalConstantBuffer->Map());

        mObjectBufferData.SetModelMatrix(Matrix::Identity());
        mObjectBuffer = mGAPI->CreateBuffer({
            .type = gapi::BufferType::Constant,
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = sizeof(ObjectBufferData),
            .debugName = CUBE_T("ObjectBuffer")
        });
        mObjectBufferDataPointer = static_cast<Uint8*>(mObjectBuffer->Map());
        memcpy(mObjectBufferDataPointer, &mObjectBufferData, sizeof(ObjectBufferData));

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

        mGAPI->WaitForGPU();

        ClearResources();

        mViewport = nullptr;

        mCommandList = nullptr;

        mObjectBuffer = nullptr;
        mGlobalConstantBuffer = nullptr;

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
        SetGlobalConstantBuffers();

        mViewport->AcquireNextImage();

        mGAPI->OnBeforeRender();
        mTextureManager.MoveNextFrame();

        RenderImpl();

        mGAPI->OnAfterRender();

        if (mRenderImGUI)
        {
            ImGui::Render();
        }

        mGAPI->OnBeforePresent(mViewport.get());
        mViewport->Present();
        mGAPI->OnAfterPresent();
    }

    void Renderer::OnResize(Uint32 width, Uint32 height)
    {
        mViewportWidth = width;
        mViewportHeight = height;

        if (mViewport)
        {
            mGAPI->WaitForGPU();

            mViewport->Resize(width, height);
        }
    }

    void Renderer::SetObjectModelMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale)
    {
        mObjectBufferData.SetModelMatrix(MatrixUtility::GetScale(scale) * MatrixUtility::GetRotationXYZ(rotation) + MatrixUtility::GetTranslation(position));
        memcpy(mObjectBufferDataPointer, &mObjectBufferData, sizeof(ObjectBufferData));
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
            mGlobalConstantBufferData.viewProjection = mViewMatrix * mPerspectiveMatrix;
            mIsViewPerspectiveMatrixDirty = false;
            updateNeeded = true;
        }

        if (mIsDirectionalLightDirty)
        {
            mGlobalConstantBufferData.directionalLightDirection = mDirectionalLightDirection;
            mIsDirectionalLightDirty = false;
            updateNeeded = true;
        }

        if (updateNeeded)
        {
            memcpy(mGlobalConstantBufferDataPointer, &mGlobalConstantBufferData, sizeof(GlobalConstantBufferData));
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
            mCommandList->SetShaderVariableConstantBuffer(0, mGlobalConstantBuffer);

            Uint32 vertexBufferOffset = 0;
            {
                // Center object
                SharedPtr<gapi::Buffer> vertexBuffer = mMesh->GetVertexBuffer();
                mCommandList->BindVertexBuffers(0, { &vertexBuffer, 1 }, { &vertexBufferOffset, 1 });
                mCommandList->BindIndexBuffer(mMesh->GetIndexBuffer(), 0);

                mCommandList->SetShaderVariableConstantBuffer(1, mObjectBuffer);

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
                            mCommandList->SetShaderVariableConstantBuffer(2, mMaterials[currentMaterialIndex]->GetMaterialBuffer());
                        }
                        else
                        {
                            mCommandList->SetShaderVariableConstantBuffer(2, mDefaultMaterial->GetMaterialBuffer());
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

                mCommandList->SetShaderVariableConstantBuffer(1, mXAxisObjectBuffer);
                mCommandList->SetShaderVariableConstantBuffer(2, mXAxisMaterial->GetMaterialBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                mCommandList->SetShaderVariableConstantBuffer(1, mYAxisObjectBuffer);
                mCommandList->SetShaderVariableConstantBuffer(2, mYAxisMaterial->GetMaterialBuffer());
                for (const SubMesh& subMesh : boxSubMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                mCommandList->SetShaderVariableConstantBuffer(1, mZAxisObjectBuffer);
                mCommandList->SetShaderVariableConstantBuffer(2, mZAxisMaterial->GetMaterialBuffer());
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
            mXAxisObjectBufferData.SetModelMatrix(MatrixUtility::GetScale(4.0f, 0.2f, 0.2f) + MatrixUtility::GetTranslation(2, 0, 0));
            mXAxisObjectBuffer = mGAPI->CreateBuffer({
                .type = gapi::BufferType::Constant,
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .size = sizeof(ObjectBufferData),
                .debugName = CUBE_T("ObjectBuffer_X")
            });
            mXAxisObjectBufferDataPointer = static_cast<Uint8*>(mXAxisObjectBuffer->Map());
            memcpy(mXAxisObjectBufferDataPointer, &mXAxisObjectBufferData, sizeof(ObjectBufferData));
            mXAxisMaterial = std::make_shared<Material>(CUBE_T("XAxisMaterial"));
            mXAxisMaterial->SetBaseColor({ 1.0f, 0.0f, 0.0f, 1.0f });

            mYAxisObjectBufferData.SetModelMatrix(MatrixUtility::GetScale(0.2f, 4.0f, 0.2f) + MatrixUtility::GetTranslation(0, 2, 0));
            mYAxisObjectBuffer = mGAPI->CreateBuffer({
                .type = gapi::BufferType::Constant,
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .size = sizeof(ObjectBufferData),
                .debugName = CUBE_T("ObjectBuffer_Y")
            });
            mYAxisObjectBufferDataPointer = static_cast<Uint8*>(mYAxisObjectBuffer->Map());
            memcpy(mYAxisObjectBufferDataPointer, &mYAxisObjectBufferData, sizeof(ObjectBufferData));
            mYAxisMaterial = std::make_shared<Material>(CUBE_T("YAxisMaterial"));
            mYAxisMaterial->SetBaseColor({ 0.0f, 1.0f, 0.0f, 1.0f });
            
            mZAxisObjectBufferData.SetModelMatrix(MatrixUtility::GetScale(0.2f, 0.2f, 4.0f) + MatrixUtility::GetTranslation(0, 0, 2));
            mZAxisObjectBuffer = mGAPI->CreateBuffer({
                .type = gapi::BufferType::Constant,
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .size = sizeof(ObjectBufferData),
                .debugName = CUBE_T("ObjectBuffer_Z")
            });
            mZAxisObjectBufferDataPointer = static_cast<Uint8*>(mZAxisObjectBuffer->Map());
            memcpy(mZAxisObjectBufferDataPointer, &mZAxisObjectBufferData, sizeof(ObjectBufferData));
            mZAxisMaterial = std::make_shared<Material>(CUBE_T("ZAxisMaterial"));
            mZAxisMaterial->SetBaseColor({ 0.0f, 0.0f, 1.0f, 1.0f });
        }
    }

    void Renderer::ClearResources()
    {
        mZAxisMaterial = nullptr;
        mZAxisObjectBuffer = nullptr;
        mYAxisMaterial = nullptr;
        mYAxisObjectBuffer = nullptr;
        mXAxisMaterial = nullptr;
        mXAxisObjectBuffer = nullptr;

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
