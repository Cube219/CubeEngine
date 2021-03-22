cmake_minimum_required (VERSION 3.6)

set(INCLUDE
    Interface/BlendState.h
    Interface/Buffer.h
    Interface/CommandList.h
    Interface/DepthStencilState.h
    Interface/Fence.h
    Interface/Framebuffer.h
    Interface/InputLayout.h
    Interface/PipelineState.h
    Interface/RasterizerState.h
    Interface/RenderPass.h
    Interface/RenderTypes.h
    Interface/Resource.h
    Interface/Sampler.h
    Interface/Shader.h
    Interface/ShaderVariables.h
    Interface/SwapChain.h
    Interface/Texture.h
    Interface/TextureView.h
    Utilities/VariableSizeMemoryPage.h
    RenderAPI.h
    RenderAPIFwd.h
    RenderAPIHeader.h
)
set(SOURCE
    a.cpp
)

# Target
add_library(CE-RenderAPI STATIC ${INCLUDE} ${SOURCE})

# Internal libs
target_link_libraries(CE-RenderAPI PUBLIC CE-Utility)
