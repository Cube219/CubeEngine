#include "AccelerationStructure.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

namespace cube
{
    SharedPtr<BLASResource> BLASResource::Create(const gapi::BLASCreateInfo& createInfo)
    {
        return std::make_shared<BLASResource>(createInfo);
    }

    BLASResource::BLASResource(const gapi::BLASCreateInfo& createInfo)
        : mVertexBufferToBuild(createInfo.vertexBuffer)
        , mIndexBufferToBuild(createInfo.indexBuffer)
    {
        mGAPIBLAS = Engine::GetRenderer()->GetGAPI().CreateBLAS(createInfo);
    }
} // namespace cube
