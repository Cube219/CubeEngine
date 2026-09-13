#pragma once

#include "CoreHeader.h"

#include "Blob.h"
#include "Buffer.h"
#include "Checker.h"
#include "RenderCore/RenderTypes.h"
#include "Vector.h"

namespace cube
{
    class BLASResource;

    namespace gapi
    {
        class BLAS;
    } // namespace gapi

    struct SubMesh
    {
        Uint64 vertexOffset;
        Uint64 numVertices;
        Uint64 indexOffset;
        Uint64 numIndices;
        int materialIndex;

        String debugName;
    };

    struct MeshMetadata
    {
        bool useFloat16 = true;
        bool remainCPUData = false;
        bool buildBLAS = false;
    };

    class MeshData
    {
    public:
        MeshData(ArrayView<Vertex> vertices, ArrayView<Index> indices, ArrayView<SubMesh> subMeshes, StringView debugName);
        ~MeshData();

        MeshData(const MeshData& other) = delete;
        MeshData& operator=(const MeshData& other) = delete;

        Uint64 GetNumVertices() const { return mNumVertices; }
        Uint64 GetNumIndices() const { return mNumIndices; }

        BlobView GetCPUVertexData() const { CHECK(mHasValidCPUDate); return mCPUData.CreateBlobView(0, sizeof(Vertex) * mNumVertices); }
        BlobView GetCPUIndexData() const { CHECK(mHasValidCPUDate); return mCPUData.CreateBlobView(mIndexOffset, sizeof(Index) * mNumIndices); }
        BlobView GetCPUData() const { CHECK(mHasValidCPUDate); return mCPUData; }

        const Vector<SubMesh>& GetSubMeshes() const { return mSubMeshes; }

        StringView GetDebugName() const { return mDebugName; }

        void ClearCPUData();

    private:
        Uint64 mNumVertices;
        Uint64 mNumIndices;
        bool mHasValidCPUDate;
        Blob mCPUData;
        Uint64 mIndexOffset;
        Vector<SubMesh> mSubMeshes;

        String mDebugName;
    };

    class Mesh
    {
    public:
        Mesh(const SharedPtr<MeshData>& meshData, const MeshMetadata& meta);
        ~Mesh();

        SharedPtr<BufferResource> GetVertexBuffer() const { return mVertexBuffer; }
        SharedPtr<BufferResource> GetIndexBuffer() const { return mIndexBuffer; }
        const Vector<SubMesh>& GetSubMeshes() const { return mMeshData->GetSubMeshes(); }

        const StringView GetDebugName() const { return mMeshData->GetDebugName(); }
        const MeshMetadata& GetMeta() const { return mMeta; }

        const SharedPtr<BLASResource> GetBLAS() const { return mBLAS; }

    private:
        friend class MeshHelper;

        SharedPtr<MeshData> mMeshData;
        MeshMetadata mMeta;

        SharedPtr<BufferResource> mVertexBuffer;
        SharedPtr<BufferResource> mIndexBuffer;

        SharedPtr<BLASResource> mBLAS;
    };
} // namespace cube
