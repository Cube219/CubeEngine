#pragma once

#include "../CoreHeader.h"

#include "RenderObject.h"
#include "../Resource/Resource.h"

#include "Utility/Vector.h"
#include "RenderAPIs/RenderAPI/Interface/Buffer.h"

namespace cube
{
    struct Vertex
    {
        Vector4 pos;
        Vector4 color;
        Vector4 normal;
        Vector2 texCoord;
    };
    using Index = Uint32;

    struct CORE_EXPORT SubMesh
    {
        Uint64 vertexOffset;
        Uint64 indexOffset;
        Uint64 numIndices;
    };

    class CORE_EXPORT MeshData
    {
    public:
        MeshData(Uint64 numVertices, Vertex* pVertices, Uint64 numIndices, Index* pIndices, Uint32 numSubMeshes, SubMesh* pSubMeshes);
        ~MeshData();

        MeshData(const MeshData& other) = delete;
        MeshData& operator=(const MeshData& other) = delete;

        Uint64 GetNumVertices() const { return mNumVertices; }
        Uint64 GetNumIndices() const { return mNumIndices; }
        void* GetData() const { return mpData; }
        Uint64 GetDataSize() const { return mDataSize; }
        const Vector<SubMesh>& GetSubMeshes() const { return mSubMeshes; }

    private:
        Uint64 mNumVertices;
        Uint64 mNumIndices;
        void* mpData;
        Uint64 mDataSize;
        Vector<SubMesh> mSubMeshes;
    };

    class CORE_EXPORT Mesh : public Resource, public RenderObject
    {
    public:
        static RPtr<Mesh> Load(StringView path);

    public:
        Mesh(const SPtr<MeshData>& meshData);
        virtual ~Mesh();

        virtual SPtr<rt::RenderObject> CreateRTObject() override;

    private:
        friend class BaseMeshGenerator;

        SPtr<MeshData> mMeshData;
    };

    namespace rt
    {
        class CORE_EXPORT Mesh : public RenderObject
        {
        public:
            Mesh(const SPtr<MeshData>& meshData);
            virtual ~Mesh();

            void OnCreate() override;
            void OnDestroy() override;

            SPtr<rapi::VertexBuffer> GetVertexBuffer() const { return mVertexBuffer; }
            SPtr<rapi::IndexBuffer> GetIndexBuffer() const { return mIndexBuffer; }
            const Vector<SubMesh>& GetSubMeshes() const { return mSubMeshes; }

        private:
            void FlushToGPU();

            SPtr<MeshData> mMeshData;

            SPtr<rapi::VertexBuffer> mVertexBuffer;
            SPtr<rapi::IndexBuffer> mIndexBuffer;
            Vector<SubMesh> mSubMeshes;
        };
    } // namespace rt
} // namespace cube
