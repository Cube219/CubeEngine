#include "Renderer.h"

#include "imgui.h"

#include "FileSystem.h"
#include "Platform.h"
#include "GAPI_Buffer.h"
#include "GAPI_CommandList.h"
#include "GAPI_Shader.h"
#include "GAPI_ShaderVariable.h"
#include "GAPI_Pipeline.h"
#include "GAPI_Viewport.h"

#include "BaseMeshGenerator.h"
#include "Checker.h"
#include "CubeMath.h"
#include "Engine.h"
#include "MatrixUtility.h"

namespace cube
{
    void Renderer::Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(LogType::Info, Renderer, "Initialize renderer. (GAPI: {})", GAPINameToString(gAPIName));

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
        mRenderImGUI = (imGUIContext.context != nullptr);

        mIsViewPerspectiveMatrixDirty = true;
        mGlobalConstantBuffer = mGAPI->CreateBuffer({
            .type = gapi::BufferType::Constant,
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = sizeof(GlobalConstantBufferData),
            .debugName = "GlobalConstantBuffer"
        });
        mGlobalConstantBufferDataPointer = static_cast<Uint8*>(mGlobalConstantBuffer->Map());

        mObjectBufferData.model = Matrix::Identity();
        mObjectBufferData.color = Vector4::Zero();
        mObjectBuffer = mGAPI->CreateBuffer({
            .type = gapi::BufferType::Constant,
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = sizeof(ObjectBufferData),
            .debugName = "ObjectBuffer"
        });
        mObjectBufferDataPointer = static_cast<Uint8*>(mObjectBuffer->Map());
        memcpy(mObjectBufferDataPointer, &mObjectBufferData, sizeof(ObjectBufferData));

        mCommandList = mGAPI->CreateCommandList({
            .debugName = "MainCommandList"
        });

        mViewportWidth = platform::Platform::GetWindowWidth();
        mViewportHeight = platform::Platform::GetWindowHeight();
        mViewport = mGAPI->CreateViewport({
            .width = mViewportWidth,
            .height = mViewportHeight,
            .vsync = true,
            .backbufferCount = 2,
            .debugName = "MainViewport"
        });

        LoadResources();
    }

    void Renderer::Shutdown(const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(LogType::Info, Renderer, "Shutdown renderer.");

        mGAPI->WaitForGPU();

        ClearResources();

        mViewport = nullptr;

        mCommandList = nullptr;

        mObjectBuffer = nullptr;
        mGlobalConstantBuffer = nullptr;

        mGAPI->Shutdown(imGUIContext);
        mGAPI = nullptr;
        mGAPI_DLib = nullptr;
    }

    void Renderer::Render()
    {
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

    void Renderer::SetGlobalConstantBuffers()
    {
        if (mIsViewPerspectiveMatrixDirty)
        {
            mGlobalConstantBufferData.viewProjection = mViewMatrix * mPerspectiveMatrix;
            mIsViewPerspectiveMatrixDirty = false;

            memcpy(mGlobalConstantBufferDataPointer, &mGlobalConstantBufferData, sizeof(GlobalConstantBufferData));
        }
    }

    void Renderer::RenderImpl()
    {
        mCommandList->Reset();
        {
            mCommandList->Begin();

            mCommandList->InsertTimestamp(CUBE_T("Begin"));

            mCommandList->SetViewports(1, &mViewport);
            gapi::ScissorRect scissor = {
                .x = 0,
                .y = 0,
                .width = mViewportWidth,
                .height = mViewportHeight
            };
            mCommandList->SetScissors(1, &scissor);
            mCommandList->SetPrimitiveTopology(gapi::PrimitiveTopology::TriangleList);

            mCommandList->ResourceTransition(mViewport, gapi::ResourceStateFlag::Present, gapi::ResourceStateFlag::RenderTarget);

            mCommandList->SetShaderVariablesLayout(mShaderVariablesLayout);
            mCommandList->SetRenderTarget(mViewport);
            mCommandList->ClearRenderTargetView(mViewport, { 0.2f, 0.2f, 0.2f, 1.0f });
            mCommandList->ClearDepthStencilView(mViewport, 0);
            mCommandList->SetGraphicsPipeline(mHelloWorldPipeline);
            mCommandList->SetShaderVariableConstantBuffer(0, mGlobalConstantBuffer);
            mCommandList->SetShaderVariableConstantBuffer(1, mObjectBuffer);

            Uint32 vertexBufferOffset = 0;
            SharedPtr<gapi::Buffer> vertexBuffer = mBoxMesh->GetVertexBuffer();
            mCommandList->BindVertexBuffers(0, 1, &vertexBuffer, &vertexBufferOffset);
            mCommandList->BindIndexBuffer(mBoxMesh->GetIndexBuffer(), 0);
            const Vector<SubMesh>& subMeshes = mBoxMesh->GetSubMeshes();
            for(const SubMesh& subMesh : subMeshes)
            {
                mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
            }

            {
                mCommandList->SetShaderVariableConstantBuffer(1, mObjectBuffer_X);
                for (const SubMesh& subMesh : subMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                mCommandList->SetShaderVariableConstantBuffer(1, mObjectBuffer_Y);
                for (const SubMesh& subMesh : subMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
                mCommandList->SetShaderVariableConstantBuffer(1, mObjectBuffer_Z);
                for (const SubMesh& subMesh : subMeshes)
                {
                    mCommandList->DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                }
            }

            mCommandList->ResourceTransition(mViewport, gapi::ResourceStateFlag::RenderTarget, gapi::ResourceStateFlag::Present);

            mCommandList->InsertTimestamp(CUBE_T("End"));

            mCommandList->End();
        }
        mCommandList->Submit();
    }

    void Renderer::LoadResources()
    {
        mBoxMesh = std::make_shared<Mesh>(BaseMeshGenerator::GetBoxMeshData());

        FrameAnsiString shaderCode;
        {
            FrameString vertexShaderFilePath = FrameString(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/HelloWorldVS.hlsl");
            SharedPtr<platform::File> vertexShaderFile = platform::FileSystem::OpenFile(vertexShaderFilePath, platform::FileAccessModeFlag::Read);
            CHECK(vertexShaderFile);
            Uint64 vertexShaderFileSize = vertexShaderFile->GetFileSize();

            shaderCode.resize(vertexShaderFileSize);
            Uint64 readSize = vertexShaderFile->Read(shaderCode.data(), vertexShaderFileSize);
            CHECK(readSize <= vertexShaderFileSize);

            mVertexShader = mGAPI->CreateShader({
                .type = gapi::ShaderType::Vertex,
                .language = gapi::ShaderLanguage::HLSL,
                .code = shaderCode.c_str(),
                .entryPoint = "VSMain",
                .debugName = "HelloWorldVS"
            });
        }
        {
            FrameString pixelShaderFilePath = FrameString(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/HelloWorldPS.hlsl");
            SharedPtr<platform::File> pixelShaderFile = platform::FileSystem::OpenFile(pixelShaderFilePath, platform::FileAccessModeFlag::Read);
            CHECK(pixelShaderFile);
            Uint64 pixelShaderFileSize = pixelShaderFile->GetFileSize();

            shaderCode.resize(pixelShaderFileSize);
            Uint64 readSize = pixelShaderFile->Read(shaderCode.data(), pixelShaderFileSize);
            CHECK(readSize <= pixelShaderFileSize);

            mPixelShader = mGAPI->CreateShader({
                .type = gapi::ShaderType::Pixel,
                .language = gapi::ShaderLanguage::HLSL,
                .code = shaderCode.c_str(),
                .entryPoint = "PSMain",
                .debugName = "HelloWorldPS"
            });
        }
        {
            mShaderVariablesLayout = mGAPI->CreateShaderVariablesLayout({
                .numShaderVariablesConstantBuffer = 2,
                .shaderVariablesConstantBuffer = nullptr,
                .debugName = "ShaderVariablesLayout"
            });
        }
        {
            gapi::InputElement inputLayout[] = {
                {
                    .name = "POSITION",
                    .format = gapi::ElementFormat::RGB32_Float,
                    .offset = 0,
                },
                {
                    .name = "COLOR",
                    .format = gapi::ElementFormat::RGBA32_Float,
                    .offset = 12,
                }
            };

            mHelloWorldPipeline = mGAPI->CreateGraphicsPipeline({
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
                .debugName = "HelloWorldPipeline"
            });
        }
        {
            mObjectBufferData_X.model = MatrixUtility::GetScale(4.0f, 0.2f, 0.2f) + MatrixUtility::GetTranslation(2, 0, 0);
            mObjectBufferData_X.color = Vector4(1, 0, 0, 1);
            mObjectBuffer_X = mGAPI->CreateBuffer({
                .type = gapi::BufferType::Constant,
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .size = sizeof(ObjectBufferData),
                .debugName = "ObjectBuffer_X"
            });
            mObjectBufferDataPointer_X = static_cast<Uint8*>(mObjectBuffer_X->Map());
            memcpy(mObjectBufferDataPointer_X, &mObjectBufferData_X, sizeof(ObjectBufferData));

            mObjectBufferData_Y.model = MatrixUtility::GetScale(0.2f, 4.0f, 0.2f) + MatrixUtility::GetTranslation(0, 2, 0);
            mObjectBufferData_Y.color = Vector4(0, 1, 0, 1);
            mObjectBuffer_Y = mGAPI->CreateBuffer({
                .type = gapi::BufferType::Constant,
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .size = sizeof(ObjectBufferData),
                .debugName = "ObjectBuffer_Y"
            });
            mObjectBufferDataPointer_Y = static_cast<Uint8*>(mObjectBuffer_Y->Map());
            memcpy(mObjectBufferDataPointer_Y, &mObjectBufferData_Y, sizeof(ObjectBufferData));

            mObjectBufferData_Z.model = MatrixUtility::GetScale(0.2f, 0.2f, 4.0f) + MatrixUtility::GetTranslation(0, 0, 2);
            mObjectBufferData_Z.color = Vector4(0, 0, 1, 1);
            mObjectBuffer_Z = mGAPI->CreateBuffer({
                .type = gapi::BufferType::Constant,
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .size = sizeof(ObjectBufferData),
                .debugName = "ObjectBuffer_Z"
            });
            mObjectBufferDataPointer_Z = static_cast<Uint8*>(mObjectBuffer_Z->Map());
            memcpy(mObjectBufferDataPointer_Z, &mObjectBufferData_Z, sizeof(ObjectBufferData));
        }
    }

    void Renderer::ClearResources()
    {
        mObjectBuffer_Z = nullptr;
        mObjectBuffer_Y = nullptr;
        mObjectBuffer_X = nullptr;

        mHelloWorldPipeline = nullptr;
        mShaderVariablesLayout = nullptr;
        mPixelShader = nullptr;
        mVertexShader = nullptr;
        mBoxMesh = nullptr;
    }
} // namespace cube
