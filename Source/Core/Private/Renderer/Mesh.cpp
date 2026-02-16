#include "Renderer/Mesh.h"


#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_Buffer.h"
#include "Platform.h"
#include "Renderer.h"

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

    ArrayView<gapi::InputElement> Mesh::GetInputElements(const MeshMetadata& meta)
    {
        if (meta.useFloat16)
        {
            constexpr int positionOffset = 0;
            constexpr int colorOffset = positionOffset + sizeof(Uint16) * 4;
            constexpr int normalOffset = colorOffset + sizeof(Uint16) * 4;
            constexpr int tangentOffset = normalOffset + sizeof(Uint16) * 4;
            constexpr int uvOffset = tangentOffset + sizeof(Uint16) * 4;

            static gapi::InputElement inputLayoutFP16[] = {
                {
                    .name = "POSITION",
                    .index = 0,
                    .format = gapi::ElementFormat::RGBA16_Float,
                    .offset = positionOffset,
                },
                {
                    .name = "COLOR",
                    .index = 0,
                    .format = gapi::ElementFormat::RGBA16_Float,
                    .offset = colorOffset,
                },
                {
                    .name = "NORMAL",
                    .index = 0,
                    .format = gapi::ElementFormat::RGBA16_Float,
                    .offset = normalOffset
                },
                {
                    .name = "TANGENT",
                    .index = 0,
                    .format = gapi::ElementFormat::RGBA16_Float,
                    .offset = tangentOffset
                },
                {
                    .name = "TEXCOORD",
                    .index = 0,
                    .format = gapi::ElementFormat::RG16_Float,
                    .offset = uvOffset
                }
            };

            return inputLayoutFP16;
        }

        constexpr int positionOffset = 0;
        constexpr int colorOffset = positionOffset + sizeof(Vertex::position);
        constexpr int normalOffset = colorOffset + sizeof(Vertex::color);
        constexpr int tangentOffset = normalOffset + sizeof(Vertex::normal);
        constexpr int uvOffset = tangentOffset + sizeof(Vertex::tangent);

        static gapi::InputElement inputLayout[] = {
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
                .name = "TANGENT",
                .index = 0,
                .format = gapi::ElementFormat::RGBA32_Float,
                .offset = tangentOffset
            },
            {
                .name = "TEXCOORD",
                .index = 0,
                .format = gapi::ElementFormat::RG32_Float,
                .offset = uvOffset
            }
        };

        return inputLayout;
    }

    Mesh::Mesh(const SharedPtr<MeshData>& meshData, const MeshMetadata& meta) :
        mMeshData(meshData),
        mMeta(meta)
    {
        GAPI& gAPI = Engine::GetRenderer()->GetGAPI();
        {
            using namespace gapi;

            Uint64 vertexBufferSize;
            Uint32 vertexStride;
            if (mMeta.useFloat16)
            {
                vertexBufferSize = sizeof(VertexFP16) * meshData->GetNumVertices();
                vertexStride = sizeof(VertexFP16);
            }
            else
            {
                vertexBufferSize = sizeof(Vertex) * meshData->GetNumVertices();
                vertexStride = sizeof(Vertex);
            }

            FrameString vbDebugName = Format<FrameString>(CUBE_T("[{0}] VertexBuffer"), meshData->GetDebugName());
            BufferCreateInfo vertexBufferCreateInfo = {
                .type = BufferType::Vertex,
                .usage = ResourceUsage::GPUOnly,
                .size = vertexBufferSize,
                .vertexStride = vertexStride,
                .debugName = vbDebugName
            };
            mVertexBuffer = gAPI.CreateBuffer(vertexBufferCreateInfo);

            FrameString ibDebugName = Format<FrameString>(CUBE_T("[{0}] IndexBuffer"), meshData->GetDebugName());
            BufferCreateInfo indexBufferCreateInfo = {
                .type = BufferType::Index,
                .usage = ResourceUsage::GPUOnly,
                .size = sizeof(Index) * meshData->GetNumIndices(),
                .debugName = ibDebugName
            };
            mIndexBuffer = gAPI.CreateBuffer(indexBufferCreateInfo);

            void* pVertexBufferData = mVertexBuffer->Map();
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
                memcpy(pVertexBufferData, vertexData.GetData(), vertexData.GetSize());
            }
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
