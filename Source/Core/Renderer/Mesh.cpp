#include "Mesh.h"

#include "RendererManager.h"
#include "../Resource/ResourceManager.h"

namespace cube
{
    MeshData::MeshData(Uint64 numVertices, Vertex* pVertices, Uint64 numIndices, Index* pIndices,
        Uint32 numSubMeshes, SubMesh* pSubMeshes) :
        mNumVertices(numVertices), mNumIndices(numIndices)
    {
        mDataSize = sizeof(Vertex) * mNumVertices + sizeof(Index) * mNumIndices;
        mpData = malloc(mDataSize);

        mSubMeshes.reserve(numSubMeshes);
        for(Uint32 i = 0; i < numSubMeshes; ++i) {
            mSubMeshes.push_back(pSubMeshes[i]);
        }
    }

    MeshData::~MeshData()
    {
        if(mpData != nullptr) free(mpData);
    }

    RPtr<Mesh> Mesh::Load(StringView path)
    {
        RPtr<Mesh> mesh = ResourceManager::LoadResource<Mesh>(path);
        mesh->OnCreate();

        return mesh;
    }

    Mesh::Mesh(const SPtr<MeshData>& meshData) :
        mMeshData(meshData)
    {}

    Mesh::~Mesh()
    {}

    SPtr<rt::RenderObject> Mesh::CreateRTObject()
    {
        // Move mesh data into rt::Mesh
        SPtr<rt::Mesh> rtMesh = std::make_shared<rt::Mesh>(mMeshData);
        mMeshData = nullptr;

        return rtMesh;
    }

    namespace rt
    {
        Mesh::Mesh(const SPtr<MeshData>& meshData) :
            mMeshData(meshData),
            mSubMeshes(meshData->GetSubMeshes())
        {}

        Mesh::~Mesh()
        {}

        void Mesh::OnCreate()
        {
            if(mMeshData != nullptr) {
                FlushToGPU();
            }
        }

        void Mesh::OnDestroy()
        {
            mIndexBuffer = nullptr;
            mVertexBuffer = nullptr;
            mMeshData = nullptr;
        }

        void Mesh::FlushToGPU()
        {
            auto& rAPI = RendererManager::GetRenderAPI();

            rapi::VertexBufferCreateInfo vbCreateInfo;
            vbCreateInfo.usage = rapi::ResourceUsage::Default;
            vbCreateInfo.size = mMeshData->GetDataSize();
            vbCreateInfo.pData = mMeshData->GetData();
            vbCreateInfo.debugName = "Mesh vertex buffer";

            mVertexBuffer = rAPI.CreateVertexBuffer(vbCreateInfo);

            rapi::IndexBufferCreateInfo ibCreateInfo;
            ibCreateInfo.usage = rapi::ResourceUsage::Default;
            ibCreateInfo.size = mMeshData->GetDataSize();
            ibCreateInfo.pData = mMeshData->GetData();
            ibCreateInfo.strideType = rapi::IndexBufferCreateInfo::StrideType::Uint32;
            ibCreateInfo.debugName = "Mesh vertex buffer";

            mIndexBuffer = rAPI.CreateIndexBuffer(ibCreateInfo);

            mMeshData = nullptr;
        }
    } // namespace rt
} // namespace cube
