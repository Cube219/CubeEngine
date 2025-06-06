#include "Mesh.h"

#include "GAPI_Buffer.h"
#include "Platform.h"

#include "Engine.h"
#include "Renderer.h"

namespace cube
{
    MeshData::MeshData(Uint64 numVertices, Vertex* pVertices, Uint64 numIndices, Index* pIndices, Uint32 numSubMeshes, SubMesh* pSubMeshes) :
        mNumVertices(numVertices),
        mNumIndices(numIndices)
    {
        mDataSize = sizeof(Vertex) * mNumVertices + sizeof(Index) * mNumIndices;
        mIndexOffset = sizeof(Vertex) * mNumVertices;
        mpData = platform::Platform::Allocate(mDataSize);

        memcpy(mpData, pVertices, sizeof(Vertex) * mNumVertices);
        memcpy((char*)mpData + mIndexOffset, pIndices, sizeof(Index) * mNumIndices);

        mSubMeshes.reserve(numSubMeshes);
        for (Uint32 i = 0; i < numSubMeshes; ++i)
        {
            mSubMeshes.push_back(pSubMeshes[i]);
        }
    }

    MeshData::~MeshData()
    {
        if (mpData != nullptr)
        {
            platform::Platform::Free(mpData);
        }
    }

    Mesh::Mesh(const SharedPtr<MeshData>& meshData) :
        mMeshData(meshData)
    {
        GAPI& gAPI = Engine::GetRenderer()->GetGAPI();
        {
            using namespace gapi;
            BufferCreateInfo vertexBufferCreateInfo = {
                .type = BufferType::Vertex,
                .usage = ResourceUsage::GPUOnly,
                .size = sizeof(Vertex) * meshData->GetNumVertices(),
                .debugName = "Vertex buffer"
            };
            mVertexBuffer = gAPI.CreateBuffer(vertexBufferCreateInfo);

            BufferCreateInfo indexBufferCreateInfo = {
                .type = BufferType::Index,
                .usage = ResourceUsage::GPUOnly,
                .size = sizeof(Index) * meshData->GetNumIndices(),
                .debugName = "Index buffer"
            };
            mIndexBuffer = gAPI.CreateBuffer(indexBufferCreateInfo);

            void* pVertexBufferData = mVertexBuffer->Map();
            memcpy(pVertexBufferData, meshData->GetVertexData(), meshData->GetVertexDataSize());
            mVertexBuffer->Unmap();

            void* pIndexBufferData = mIndexBuffer->Map();
            memcpy(pIndexBufferData, meshData->GetIndexData(), meshData->GetIndexDataSize());
            mIndexBuffer->Unmap();
        }
    }

    Mesh::~Mesh()
    {
        mIndexBuffer = nullptr;
        mVertexBuffer = nullptr;
    }
} // namespace cube
