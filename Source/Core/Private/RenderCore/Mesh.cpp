#include "Mesh.h"


#include "Allocator/FrameAllocator.h"
#include "BufferManager.h"
#include "Engine.h"
#include "GAPI_Buffer.h"
#include "Platform.h"
#include "Renderer/Renderer.h"

namespace cube
{
    MeshData::MeshData(ArrayView<Vertex> vertices, ArrayView<Index> indices, ArrayView<SubMesh> subMeshes, StringView debugName) :
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

    Mesh::Mesh(const SharedPtr<MeshData>& meshData, const MeshMetadata& meta) :
        mMeshData(meshData),
        mMeta(meta)
    {
        GAPI& gAPI = Engine::GetRenderer()->GetGAPI();
        {
            using namespace gapi;

            Uint64 vertexBufferSize;
            if (mMeta.useFloat16)
            {
                vertexBufferSize = sizeof(VertexFP16) * meshData->GetNumVertices();
            }
            else
            {
                vertexBufferSize = sizeof(VertexFP32) * meshData->GetNumVertices();
            }

            FrameString vbDebugName = Format<FrameString>(CUBE_T("[{0}] VertexBuffer"), meshData->GetDebugName());
            BufferCreateInfo vertexBufferCreateInfo = {
                .usage = ResourceUsage::GPUOnly,
                .bufferInfo = {
                    .type = BufferType::Raw,
                    .size = vertexBufferSize,
                },
                .debugName = vbDebugName
            };
            mVertexBuffer = gAPI.CreateBuffer(vertexBufferCreateInfo);

            FrameString ibDebugName = Format<FrameString>(CUBE_T("[{0}] IndexBuffer"), meshData->GetDebugName());
            BufferCreateInfo indexBufferCreateInfo = {
                .usage = ResourceUsage::GPUOnly,
                .bufferInfo = {
                    .type = BufferType::Typed,
                    .size = sizeof(Index) * meshData->GetNumIndices(),
                    .stride = sizeof(Index),
                },
                .debugName = ibDebugName
            };
            mIndexBuffer = gAPI.CreateBuffer(indexBufferCreateInfo);

            UploadManager& uploadManager = Engine::GetRenderer()->GetUploadManager();

            UploadDesc vbUploadDesc = uploadManager.Allocate(mVertexBuffer, true);
            void* pVertexBufferData = vbUploadDesc.pData;
            if (mMeta.useFloat16)
            {
                BlobView vertexData = meshData->GetVertexData();
                const Vertex* vertices = reinterpret_cast<const Vertex*>(vertexData.GetData());
                VertexFP16* fp16Vertices = reinterpret_cast<VertexFP16*>(pVertexBufferData);
                for (Uint64 i = 0; i < meshData->GetNumVertices(); ++i)
                {
                    fp16Vertices[i] = ConvertVertexToFP16(vertices[i]);
                }
            }
            else
            {
                BlobView vertexData = meshData->GetVertexData();
                const Vertex* vertices = reinterpret_cast<const Vertex*>(vertexData.GetData());
                VertexFP32* fp32Vertices = reinterpret_cast<VertexFP32*>(pVertexBufferData);
                for (Uint64 i = 0; i < meshData->GetNumVertices(); ++i)
                {
                    fp32Vertices[i] = ConvertVertexToFP32(vertices[i]);
                }
            }

            BufferManager& bufferManager = Engine::GetRenderer()->GetBufferManager();
            SharedPtr<gapi::CommandList> bufferInitCommandList = bufferManager.GetBufferInitCommandList();
            SharedPtr<gapi::Fence> bufferInitFence = bufferManager.GetBufferInitFence();

            Uint64 fenceValue = bufferManager.GetAndMoveBufferInitFenceValue();
            uploadManager.SubmitToCopyQueue(vbUploadDesc, bufferInitFence, fenceValue);

            UploadDesc ibUploadDesc = uploadManager.Allocate(mIndexBuffer, true);
            void* pIndexBufferData = ibUploadDesc.pData;
            BlobView indexData = meshData->GetIndexData();
            memcpy(pIndexBufferData, indexData.GetData(), indexData.GetSize());

            fenceValue = bufferManager.GetAndMoveBufferInitFenceValue();
            uploadManager.SubmitToCopyQueue(ibUploadDesc, bufferInitFence, fenceValue);

            mInitFenceValue = fenceValue;
        }
    }

    Mesh::~Mesh()
    {
        mIndexBuffer = nullptr;
        mVertexBuffer = nullptr;
    }

    void Mesh::WaitUntilInitialized()
    {
        if (mIsInitialized)
        {
            return;
        }

        CheckInitialized();

        if (!mIsInitialized)
        {
            BufferManager& bufferManager = Engine::GetRenderer()->GetBufferManager();
            bufferManager.GetBufferInitFence()->Wait(mInitFenceValue);
        }
    }

    void Mesh::WaitUntilInitialized(gapi::CommandList& commandList)
    {
        if (mIsInitialized)
        {
            return;
        }

        CheckInitialized();

        if (!mIsInitialized)
        {
            BufferManager& bufferManager = Engine::GetRenderer()->GetBufferManager();
            commandList.WaitForFence(bufferManager.GetBufferInitFence(), mInitFenceValue);
        }
    }

    void Mesh::CheckInitialized()
    {
        if (mIsInitialized)
        {
            return;
        }

        BufferManager& bufferManager = Engine::GetRenderer()->GetBufferManager();
        mIsInitialized = (bufferManager.GetBufferInitFence()->GetCompletedValue() >= mInitFenceValue);
    }
} // namespace cube
