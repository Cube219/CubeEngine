#include "Renderer/Mesh.h"


#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_Buffer.h"
#include "Platform.h"
#include "Renderer.h"

namespace cube
{
    MeshData::MeshData(ArrayView<Vertex> vertices, ArrayView<Index> indices, ArrayView<SubMesh> subMeshes, AnsiStringView debugName) :
        mNumVertices(vertices.size()),
        mNumIndices(indices.size()),
        mDebugName(debugName)
    {
        Uint64 dataSize = sizeof(Vertex) * mNumVertices + sizeof(Index) * mNumIndices;
        mIndexOffset = sizeof(Vertex) * mNumVertices;
        mData = Blob(dataSize);
        memcpy(mData.GetData(), vertices.data(), sizeof(Vertex) * mNumVertices);
        memcpy((Byte*)mData.GetData() + mIndexOffset, indices.data(), sizeof(Index) * mNumIndices);

        mSubMeshes = Vector<SubMesh>(subMeshes.begin(), subMeshes.end());
    }

    MeshData::~MeshData()
    {
    }

    Mesh::Mesh(const SharedPtr<MeshData>& meshData) :
        mMeshData(meshData)
    {
        GAPI& gAPI = Engine::GetRenderer()->GetGAPI();
        {
            using namespace gapi;
            FrameAnsiString vbDebugName = Format<FrameAnsiString>("{0}-Vertex buffer", meshData->GetDebugName());
            BufferCreateInfo vertexBufferCreateInfo = {
                .type = BufferType::Vertex,
                .usage = ResourceUsage::GPUOnly,
                .size = sizeof(Vertex) * meshData->GetNumVertices(),
                .debugName = vbDebugName.c_str()
            };
            mVertexBuffer = gAPI.CreateBuffer(vertexBufferCreateInfo);

            FrameAnsiString ibDebugName = Format<FrameAnsiString>("{0}-Index buffer", meshData->GetDebugName());
            BufferCreateInfo indexBufferCreateInfo = {
                .type = BufferType::Index,
                .usage = ResourceUsage::GPUOnly,
                .size = sizeof(Index) * meshData->GetNumIndices(),
                .debugName = ibDebugName.c_str()
            };
            mIndexBuffer = gAPI.CreateBuffer(indexBufferCreateInfo);

            void* pVertexBufferData = mVertexBuffer->Map();
            BlobView vertexData = meshData->GetVertexData();
            memcpy(pVertexBufferData, vertexData.GetData(), vertexData.GetSize());
            mVertexBuffer->Unmap();

            void* pIndexBufferData = mIndexBuffer->Map();
            BlobView indexData = meshData->GetIndexData();
            memcpy(pIndexBufferData, indexData.GetData(), indexData.GetSize());
            mIndexBuffer->Unmap();
        }
    }

    Mesh::~Mesh()
    {
        mIndexBuffer = nullptr;
        mVertexBuffer = nullptr;
    }
} // namespace cube
