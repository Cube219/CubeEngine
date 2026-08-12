#include "Mesh.h"


#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_Buffer.h"
#include "GAPI_CommandList.h"
#include "Platform.h"
#include "Renderer/Renderer.h"
#include "RenderCore/ResourceManager.h"
#include "RenderCore/RenderGraph.h"
#include "UploadManager.h"

namespace cube
{
    MeshData::MeshData(ArrayView<Vertex> vertices, ArrayView<Index> indices, ArrayView<SubMesh> subMeshes, StringView debugName)
        : mNumVertices(vertices.size())
        , mNumIndices(indices.size())
        , mDebugName(debugName)
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

    Mesh::Mesh(const SharedPtr<MeshData>& meshData, const MeshMetadata& meta)
        : mMeshData(meshData)
        , mMeta(meta)
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
            mVertexBuffer = std::make_shared<BufferResource>(vertexBufferCreateInfo);

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
            mIndexBuffer = std::make_shared<BufferResource>(indexBufferCreateInfo);

            UploadManager& uploadManager = Engine::GetRenderer()->GetUploadManager();
            ResourceManager& resourceManager = Engine::GetRenderer()->GetResourceManager();

            UploadDesc vbUploadDesc = uploadManager.Allocate(mVertexBuffer->GetGAPIBuffer(), true);
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
            Uint64 finishVBFenceValue = uploadManager.SubmitToCopyQueue(vbUploadDesc);

            UploadDesc ibUploadDesc = uploadManager.Allocate(mIndexBuffer->GetGAPIBuffer(), true);
            void* pIndexBufferData = ibUploadDesc.pData;
            BlobView indexData = meshData->GetIndexData();
            memcpy(pIndexBufferData, indexData.GetData(), indexData.GetSize());
            Uint64 finishIBFenceValue = uploadManager.SubmitToCopyQueue(ibUploadDesc);

            Uint64 finishFenceValue = Math::Max(finishVBFenceValue, finishIBFenceValue);

            if (finishFenceValue > 0)
            {
                resourceManager.QueuePreprocessTask([vertexBuffer = mVertexBuffer, indexBuffer = mIndexBuffer, finishFenceValue](RGBuilder& builder)
                {
                    builder.AddPass(CUBE_T("##Preprocess - Wait mesh upload"),
                    [vertexBuffer, indexBuffer, finishFenceValue](gapi::CommandList& commandList)
                    {
                        UploadManager& uploadManager = Engine::GetRenderer()->GetUploadManager();
                        commandList.WaitForFence(uploadManager.GetFinishFence(), finishFenceValue);
                    });
                });
            }
        }
    }

    Mesh::~Mesh()
    {
        mIndexBuffer = nullptr;
        mVertexBuffer = nullptr;
    }
} // namespace cube
