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
#include "Engine.h"

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

        ClearResources();

        mViewport = nullptr;

        mCommandList = nullptr;

        mGAPI->Shutdown(imGUIContext);
        mGAPI = nullptr;
        mGAPI_DLib = nullptr;
    }

    void Renderer::Render()
    {
        mViewport->AcquireNextImage();

        mGAPI->OnBeforeRender();
        RenderImpl();
        mGAPI->OnAfterRender();

        ImGui::Render();

        mGAPI->OnBeforePresent(mViewport.get());
        mViewport->Present();
        mGAPI->OnAfterPresent();
    }

    void Renderer::OnResize(Uint32 width, Uint32 height)
    {
        if (mViewport)
        {
            mViewport->Resize(width, height);
        }
    }

    void Renderer::RenderImpl()
    {
        mCommandList->Reset();
        {
            mCommandList->Begin();

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

            mCommandList->SetShaderVariablesLayout(mEmptyShaderVariablesLayout);
            mCommandList->SetRenderTarget(mViewport);
            mCommandList->ClearRenderTargetView(mViewport, { 0.2f, 0.2f, 0.2f, 1.0f });
            mCommandList->SetGraphicsPipeline(mHelloWorldPipeline);

            Uint32 vertexBufferOffset = 0;
            mCommandList->BindVertexBuffers(0, 1, &mTriangleVertexBuffer, &vertexBufferOffset);
            mCommandList->Draw(3, 0);

            mCommandList->ResourceTransition(mViewport, gapi::ResourceStateFlag::RenderTarget, gapi::ResourceStateFlag::Present);

            mCommandList->End();
        }
        mCommandList->Submit();
    }

    void Renderer::LoadResources()
    {
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0, 0, 0, 0 }, { 0, 0 } },
            { { 0.25f, -0.25f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0, 0, 0, 0 }, { 0, 0 } },
            { { -0.25f, -0.25f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0, 0, 0, 0 }, { 0, 0 } }
        };
        mTriangleVertexBuffer = mGAPI->CreateBuffer({
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = sizeof(triangleVertices),
            .debugName = "TriangleVertexBuffer"
        });
        void* pVertexBufferData = mTriangleVertexBuffer->Map();
        memcpy(pVertexBufferData, triangleVertices, sizeof(triangleVertices));
        mTriangleVertexBuffer->Unmap();

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
            mEmptyShaderVariablesLayout = mGAPI->CreateShaderVariablesLayout({
                .debugName = "EmptyShaderVariablesLayout"
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
                .numInputLayoutElements = _countof(inputLayout),
                .numRenderTargets = 1,
                .renderTargetFormats = { gapi::ElementFormat::RGBA8_UNorm },
                .shaderVariablesLayout = mEmptyShaderVariablesLayout,
                .debugName = "HelloWorldPipeline"
            });
        }
    }

    void Renderer::ClearResources()
    {
        mHelloWorldPipeline = nullptr;
        mEmptyShaderVariablesLayout = nullptr;
        mPixelShader = nullptr;
        mVertexShader = nullptr;
        mTriangleVertexBuffer = nullptr;
    }
} // namespace cube
