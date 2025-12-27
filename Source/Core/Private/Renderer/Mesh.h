#pragma once

#include "CoreHeader.h"

#include "Blob.h"
#include "GAPI_Pipeline.h"
#include "Renderer/RenderTypes.h"
#include "Vector.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
    } // namespace gapi

    struct SubMesh
    {
        Uint64 vertexOffset;
        Uint64 indexOffset;
        Uint64 numIndices;
        int materialIndex;

        String debugName;
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

        BlobView GetVertexData() const { return mData.CreateBlobView(0, sizeof(Vertex) * mNumVertices); }
        BlobView GetIndexData() const { return mData.CreateBlobView(mIndexOffset, sizeof(Index) * mNumIndices); }
        BlobView GetData() const { return mData; }

        const Vector<SubMesh>& GetSubMeshes() const { return mSubMeshes; }

        StringView GetDebugName() const { return mDebugName; }

    private:
        Uint64 mNumVertices;
        Uint64 mNumIndices;
        Blob mData;
        Uint64 mIndexOffset;
        Vector<SubMesh> mSubMeshes;

        String mDebugName;
    };

    class Mesh
    {
    public:
        static ArrayView<gapi::InputElement> GetInputElements();

        Mesh(const SharedPtr<MeshData>& meshData);
        ~Mesh();

        SharedPtr<gapi::Buffer> GetVertexBuffer() const { return mVertexBuffer; }
        SharedPtr<gapi::Buffer> GetIndexBuffer() const { return mIndexBuffer; }
        const Vector<SubMesh>& GetSubMeshes() const { return mMeshData->GetSubMeshes(); }

    private:
        friend class MeshHelper;

        SharedPtr<MeshData> mMeshData;

        SharedPtr<gapi::Buffer> mVertexBuffer;
        SharedPtr<gapi::Buffer> mIndexBuffer;
    };
} // namespace cube
