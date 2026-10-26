#include "Buffer.h"

#include "Engine.h"
#include "GAPI.h"
#include "GAPI_Buffer.h"
#include "Renderer/Renderer.h"

namespace cube
{
    BufferResource::BufferResource(const gapi::BufferCreateInfo& createInfo)
    {
        mBuffer = Engine::GetRenderer()->GetGAPI().CreateBuffer(createInfo);
    }
} // namespace cube
