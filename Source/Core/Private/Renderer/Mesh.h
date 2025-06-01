#pragma once

#include "CoreHeader.h"

#include "Vector.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
    }

    struct Vertex
	{
	    Vector3 position;
	    Vector4 color;
	    Vector3 normal;
	    Vector2 texCoord;
	};
	using Index = Uint32;

	struct SubMesh
	{
	    Uint64 vertexOffset;
		Uint64 indexOffset;
		Uint64 numIndices;
	};

	class MeshData
    {
    public:
        MeshData(Uint64 numVertices, Vertex* pVertices, Uint64 numIndices, Index* pIndices, Uint32 numSubMeshes, SubMesh* pSubMeshes);
        ~MeshData();

        MeshData(const MeshData& other) = delete;
        MeshData& operator=(const MeshData& other) = delete;

        Uint64 GetNumVertices() const { return mNumVertices; }
        Uint64 GetNumIndices() const { return mNumIndices; }

        void* GetVertexData() const { return mpData; }
        Uint64 GetVertexDataSize() const { return sizeof(Vertex) * mNumVertices; }
        void* GetIndexData() const { return (char*)mpData + mIndexOffset; }
        Uint64 GetIndexDataSize() const { return sizeof(Index) * mNumIndices; }
        void* GetData() const { return mpData; }
        Uint64 GetDataSize() const { return mDataSize; }

	    const Vector<SubMesh>& GetSubMeshes() const { return mSubMeshes; }

    private:
        Uint64 mNumVertices;
        Uint64 mNumIndices;
        void* mpData;
        Uint64 mDataSize;
        Uint64 mIndexOffset;
        Vector<SubMesh> mSubMeshes;
    };

    class Mesh
    {
    public:
        Mesh(const SharedPtr<MeshData>& meshData);
        ~Mesh();

        SharedPtr<gapi::Buffer> GetVertexBuffer() const { return mVertexBuffer; }
        SharedPtr<gapi::Buffer> GetIndexBuffer() const { return mIndexBuffer; }
        const Vector<SubMesh>& GetSubMeshes() const { return mMeshData->GetSubMeshes(); }

    private:
        friend class BaseMeshGenerator;

        SharedPtr<MeshData> mMeshData;

        SharedPtr<gapi::Buffer> mVertexBuffer;
        SharedPtr<gapi::Buffer> mIndexBuffer;
    };
} // namespace cube
