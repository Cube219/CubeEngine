#include "BaseMeshGenerator.h"

#include "Mesh.h"
#include "../Resource/ResourceManager.h"

#include "Utility/Math.h"

namespace cube
{
    Uint32 BaseMeshGenerator::nextTempID = 0;

    RPtr<Mesh> BaseMeshGenerator::GetBoxMesh()
    {
        FrameVector<Vertex> vertices;
        Vertex v;
        v.pos = Vector4{ -0.5f, -0.5f, -0.5f, 1 };
        v.color = Vector4{ 1, 0, 0, 1 };
        v.texCoord = { 0, 0 };
        vertices.push_back(v);
        v.pos = Vector4{ -0.5f, 0.5f, -0.5f, 1 };
        v.color = Vector4{ 0, 1, 0, 1 };
        v.texCoord = { 1, 0 };
        vertices.push_back(v);
        v.pos = Vector4{ 0.5f, -0.5f, -0.5f, 1 };
        v.color = Vector4{ 0, 0, 0, 1 };
        v.texCoord = { 0, 1 };
        vertices.push_back(v);
        v.pos = Vector4{ 0.5f, 0.5f, -0.5f, 1 };
        v.color = Vector4{ 1, 1, 0, 1 };
        v.texCoord = { 1, 1 };
        vertices.push_back(v);
        v.pos = Vector4{ 0.5f, -0.5f, 0.5f, 1 };
        v.color = Vector4{ 1, 0, 1, 1 };
        v.texCoord = { 0, 0 };
        vertices.push_back(v);
        v.pos = Vector4{ 0.5f, 0.5f, 0.5f, 1 };
        v.color = Vector4{ 0, 1, 1, 1 };
        v.texCoord = { 1, 0 };
        vertices.push_back(v);
        v.pos = Vector4{ -0.5f, -0.5f, 0.5f, 1 };
        v.color = Vector4{ 1, 1, 1, 1 };
        v.texCoord = { 0, 1 };
        vertices.push_back(v);
        v.pos = Vector4{ -0.5f, 0.5f, 0.5f, 1 };
        v.color = Vector4{ 0, 0, 0, 1 };
        v.texCoord = { 1, 1 };
        vertices.push_back(v);

        FrameVector<Index> indices = {
            0,1,2,  2,1,3 ,  2,3,4,  4,3,5,  4,5,6,  6,5,7,  6,7,0,  0,7,1,  6,0,2,  2,4,6,  3,1,5,  5,1,7
        };

        SetNormalVector(vertices, indices);

        SubMesh subMesh;
        // subMesh.name = CUBE_T("Box");
        subMesh.vertexOffset = 0;
        subMesh.indexOffset = 0;
        subMesh.numIndices = indices.size();

        SPtr<MeshData> meshData = std::make_shared<MeshData>(vertices.size(), vertices.data(), indices.size(), indices.data(), 1, &subMesh);
        Mesh* meshPtr = new Mesh(meshData);

        return RegisterToResourceManager(meshPtr);
    }

    RPtr<Mesh> BaseMeshGenerator::GetCylinderMesh()
    {
        constexpr int sliceCount = 20;

        FrameVector<Vertex> vertices;
        vertices.resize(sliceCount * 2);

        // Side
        float dTheta = 2.0f * Math::Pi / sliceCount;
        for(int i = 0; i < sliceCount; i++) {
            // Down
            vertices[i * 2].pos = Vector4{ 0.5f * Math::Cos(dTheta * i), -1.0f, 0.5f * Math::Sin(dTheta * i), 1.0f };
            vertices[i * 2].color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
            vertices[i * 2].texCoord = { 0.0f, 0.0f };

            // Up
            vertices[i * 2 + 1].pos = Vector4{ 0.5f * Math::Cos(dTheta * i), 1.0f, 0.5f * Math::Sin(dTheta * i), 1.0f };
            vertices[i * 2 + 1].color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
            vertices[i * 2 + 1].texCoord = { 0.0f, 0.0f };
        }

        FrameVector<Index> indices;
        indices.resize(sliceCount * 6);

        for(int i = 0; i < sliceCount - 1; i++) {
            indices[i * 6] = i * 2;
            indices[i * 6 + 1] = i * 2 + 1;
            indices[i * 6 + 2] = i * 2 + 2;

            indices[i * 6 + 3] = i * 2 + 2;
            indices[i * 6 + 4] = i * 2 + 1;
            indices[i * 6 + 5] = i * 2 + 3;
        }
        indices[(sliceCount - 1) * 6] = (sliceCount - 1) * 2;
        indices[(sliceCount - 1) * 6 + 1] = (sliceCount - 1) * 2 + 1;
        indices[(sliceCount - 1) * 6 + 2] = 0;

        indices[(sliceCount - 1) * 6 + 3] = 0;
        indices[(sliceCount - 1) * 6 + 4] = (sliceCount - 1) * 2 + 1;
        indices[(sliceCount - 1) * 6 + 5] = 1;

        // Bottom
        Vertex v;
        v.pos = Vector4{ 0.0f, -1.0f, 0.0f, 1.0f };
        v.color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        v.texCoord = { 0.0f, 0.0f };
        vertices.push_back(v); // index: sliceCount * 2

        for(int i = 0; i < sliceCount - 1; i++) {
            indices.push_back(sliceCount * 2);
            indices.push_back(i * 2);
            indices.push_back((i + 1) * 2);
        }
        indices.push_back(sliceCount * 2);
        indices.push_back((sliceCount - 1) * 2);
        indices.push_back(0);

        // Top
        v.pos = Vector4{ 0.0f, 1.0f, 0.0f, 1.0f };
        v.color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        v.texCoord = { 0.0f, 0.0f };
        vertices.push_back(v); // index: sliceCount * 2 + 1

        for(int i = 0; i < sliceCount - 1; i++) {
            indices.push_back(sliceCount * 2 + 1);
            indices.push_back((i + 1) * 2 + 1);
            indices.push_back(i * 2 + 1);
        }
        indices.push_back(sliceCount * 2 + 1);
        indices.push_back(1);
        indices.push_back((sliceCount - 1) * 2 + 1);

        SetNormalVector(vertices, indices);

        SubMesh subMesh;
        // subMesh.name = CUBE_T("Cylinder");
        subMesh.vertexOffset = 0;
        subMesh.indexOffset = 0;
        subMesh.numIndices = indices.size();

        SPtr<MeshData> meshData = std::make_shared<MeshData>(vertices.size(), vertices.data(), indices.size(), indices.data(), 1, &subMesh);
        Mesh* meshPtr = new Mesh(meshData);

        return RegisterToResourceManager(meshPtr);
    }

    RPtr<Mesh> BaseMeshGenerator::GetCapsuleMesh()
    {
        //auto mesh = std::make_shared<Mesh>();
        // TODO: 차후 구현

        return nullptr;
    }

    RPtr<Mesh> BaseMeshGenerator::GetSphereMesh()
    {
        constexpr int divisionNum = 3;

        // Create icosahedron
        constexpr float x = 0.525731f;
        constexpr float z = 0.850651f;

        FrameVector<Vertex> vertices;
        Vertex v;
        v.color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        v.texCoord = { 0.0f, 0.0f };

        v.pos = Vector4{ -x, 0.0f, z, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ x, 0.0f, z, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ -x, 0.0f, -z, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ x, 0.0f, -z, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ 0.0f, z, x, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ 0.0f, z, -x, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ 0.0f, -z, x, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ 0.0f, -z, -x, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ z, x, 0.0f, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ -z, x, 0.0f, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ z, -x, 0.0f, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ -z, -x, 0.0f, 1.0f };
        vertices.push_back(v);

        FrameVector<Index> indices = {
            1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
            1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
            3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
            10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
        };

        // Divide vertices from 1 to 4
        for(int i = 0; i < divisionNum; i++) {
            SubDivide(vertices, indices);
        }

        // Project onto sphere
        for(Uint64 i = 0; i < vertices.size(); i++) {
            Vector3 v;
            v = vertices[i].pos;
            v.Normalize();
            v *= 0.5f;

            Float3 vF = v.GetFloat3();

            vertices[i].pos = Vector4(vF.x, vF.y, vF.z, 1.0f);
            vertices[i].normal = Vector3(vF.x, vF.y, vF.z);
            vertices[i].normal.Normalize();
        }

        //SetNormalVector(meshPtr);

        SubMesh subMesh;
        // subMesh.name = CUBE_T("Sphere");
        subMesh.vertexOffset = 0;
        subMesh.indexOffset = 0;
        subMesh.numIndices = indices.size();

        SPtr<MeshData> meshData = std::make_shared<MeshData>(vertices.size(), vertices.data(), indices.size(), indices.data(), 1, &subMesh);
        Mesh* meshPtr = new Mesh(meshData);

        return RegisterToResourceManager(meshPtr);
    }

    RPtr<Mesh> BaseMeshGenerator::GetPlaneMesh()
    {
        FrameVector<Vertex> vertices;
        Vertex v;
        v.color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        v.texCoord = { 0.0f, 0.0f };

        v.pos = Vector4{ -5.0f, 0.0f, -5.0f, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ -5.0f, 0.0f, 5.0f, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ 5.0f, 0.0f, 5.0f, 1.0f };
        vertices.push_back(v);
        v.pos = Vector4{ 5.0f, 0.0f, -5.0f, 1.0f };
        vertices.push_back(v);

        FrameVector<Index> indices = {
            0,1,2,  0,2,3
        };

        SetNormalVector(vertices, indices);

        SubMesh subMesh;
        // subMesh.name = CUBE_T("Plane");
        subMesh.vertexOffset = 0;
        subMesh.indexOffset = 0;
        subMesh.numIndices = indices.size();

        SPtr<MeshData> meshData = std::make_shared<MeshData>(vertices.size(), vertices.data(), indices.size(), indices.data(), 1, &subMesh);
        Mesh* meshPtr = new Mesh(meshData);

        return RegisterToResourceManager(meshPtr);
    }

    void BaseMeshGenerator::SubDivide(FrameVector<Vertex>& vertices, FrameVector<Index>& indices)
    {
        FrameVector<Vertex>& oldVertices = vertices;
        FrameVector<Index>& oldIndices = indices;
        FrameVector<Vertex> newVertices;
        FrameVector<Index> newIndices;

        //       v1
        //       *
        //      / \
		//     /   \
		//  m0*-----*m1
        //   / \   / \
		//  /   \ /   \
		// *-----*-----*
        // v0    m2     v2

        int numTriangles = (int)oldIndices.size() / 3;
        for(int i = 0; i < numTriangles; i++) {
            Vertex v0 = oldVertices[oldIndices[i * 3]];
            Vertex v1 = oldVertices[oldIndices[i * 3 + 1]];
            Vertex v2 = oldVertices[oldIndices[i * 3 + 2]];

            Vertex m0, m1, m2;
            m0.pos = (v0.pos + v1.pos) / 2.0f;

            m1.pos = (v1.pos + v2.pos) / 2.0f;

            m2.pos = (v0.pos + v2.pos) / 2.0f;

            newVertices.push_back(v0); // 0
            newVertices.push_back(v1); // 1
            newVertices.push_back(v2); // 2
            newVertices.push_back(m0); // 3
            newVertices.push_back(m1); // 4
            newVertices.push_back(m2); // 5

            newIndices.push_back(i * 6);
            newIndices.push_back(i * 6 + 3);
            newIndices.push_back(i * 6 + 5);

            newIndices.push_back(i * 6 + 3);
            newIndices.push_back(i * 6 + 4);
            newIndices.push_back(i * 6 + 5);

            newIndices.push_back(i * 6 + 5);
            newIndices.push_back(i * 6 + 4);
            newIndices.push_back(i * 6 + 2);

            newIndices.push_back(i * 6 + 3);
            newIndices.push_back(i * 6 + 1);
            newIndices.push_back(i * 6 + 4);
        }

        vertices = newVertices;
        indices = newIndices;
    }

    void BaseMeshGenerator::SetNormalVector(FrameVector<Vertex>& vertices, FrameVector<Index>& indices)
    {
        for(auto& v : vertices) {
            v.normal = Vector4{ 0.0f, 0.0f, 0.0f, 1.0f };
        }

        int numTriangles = (int)indices.size() / 3;
        for(int i = 0; i < numTriangles; i++) {
            Index i0 = indices[i * 3];
            Index i1 = indices[i * 3 + 1];
            Index i2 = indices[i * 3 + 2];
            Vertex v0 = vertices[i0];
            Vertex v1 = vertices[i1];
            Vertex v2 = vertices[i2];

            Vector3 t0;
            t0 = v1.pos - v0.pos;
            Vector3 t1;
            t1 = v2.pos - v0.pos;

            Vector3 n = Vector3::Cross(t0, t1);

            vertices[i0].normal += n;
            vertices[i1].normal += n;
            vertices[i2].normal += n;
        }

        for(auto& v : vertices) {
            v.normal.Normalize();
        }
    }

    RPtr<Mesh> BaseMeshGenerator::RegisterToResourceManager(Mesh* mesh)
    {
        FrameString tempID = FrameFormat(CUBE_T("_BaseMesh{0}"), nextTempID);
        nextTempID++;

        {
            Lock lock(ResourceManager::mLoadedResourcesMutex);

            ResourceManager::mLoadedResources[tempID.c_str()] = mesh;
        }

        mesh->OnCreate();

        return RPtr<Mesh>(mesh);
    }
} // namespace cube
