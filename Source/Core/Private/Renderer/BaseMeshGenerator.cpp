#include "BaseMeshGenerator.h"

#include "CubeMath.h"

namespace cube
{
    SharedPtr<MeshData> BaseMeshGenerator::GetBoxMeshData()
    {
        FrameVector<Vertex> vertices;
        Vertex v;
        v.position = { -0.5f, -0.5f, -0.5f, 1 };
        v.color = { 1, 0, 0, 1 };
        v.uv = { 0, 0 };
        vertices.push_back(v);
        v.position = { -0.5f, 0.5f, -0.5f, 1 };
        v.color = { 0, 1, 0, 1 };
        v.uv = { 1, 0 };
        vertices.push_back(v);
        v.position = { 0.5f, -0.5f, -0.5f, 1 };
        v.color = { 0, 0, 0, 1 };
        v.uv = { 0, 1 };
        vertices.push_back(v);
        v.position = { 0.5f, 0.5f, -0.5f, 1 };
        v.color = { 1, 1, 0, 1 };
        v.uv = { 1, 1 };
        vertices.push_back(v);
        v.position = { 0.5f, -0.5f, 0.5f, 1 };
        v.color = { 1, 0, 1, 1 };
        v.uv = { 0, 0 };
        vertices.push_back(v);
        v.position = { 0.5f, 0.5f, 0.5f, 1 };
        v.color = { 0, 1, 1, 1 };
        v.uv = { 1, 0 };
        vertices.push_back(v);
        v.position = { -0.5f, -0.5f, 0.5f, 1 };
        v.color = { 1, 1, 1, 1 };
        v.uv = { 0, 1 };
        vertices.push_back(v);
        v.position = { -0.5f, 0.5f, 0.5f, 1 };
        v.color = { 0, 0, 0, 1 };
        v.uv = { 1, 1 };
        vertices.push_back(v);

        FrameVector<Index> indices = {
            0,1,2,  2,1,3 ,  2,3,4,  4,3,5,  4,5,6,  6,5,7,  6,7,0,  0,7,1,  6,0,2,  2,4,6,  3,1,5,  5,1,7
        };

        SetNormalVector(vertices, indices);

        SubMesh subMeshes[] = {
            {
                .vertexOffset = 0,
                .indexOffset = 0,
                .numIndices = indices.size(),
                .materialIndex = 0,
                .debugName = "Main"
            }
        };

        return std::make_shared<MeshData>(vertices, indices, subMeshes, "BaseBoxMesh");
    }

    SharedPtr<MeshData> BaseMeshGenerator::GetCylinderMeshData()
    {
        constexpr int sliceCount = 20;

        FrameVector<Vertex> vertices;
        vertices.resize(sliceCount * 2);

        // Side
        float dTheta = 2.0f * Math::Pi / sliceCount;
        for (int i = 0; i < sliceCount; ++i)
        {
            // Down
            vertices[i * 2].position = { 0.5f * Math::Cos(dTheta * i), -1.0f, 0.5f * Math::Sin(dTheta * i), 1.0f };
            vertices[i * 2].color = { 1.0f, 1.0f, 1.0f, 1.0f };
            vertices[i * 2].uv = { 0.0f, 0.0f };

            // Up
            vertices[i * 2 + 1].position = { 0.5f * Math::Cos(dTheta * i), 1.0f, 0.5f * Math::Sin(dTheta * i), 1.0f };
            vertices[i * 2 + 1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
            vertices[i * 2 + 1].uv = { 0.0f, 0.0f };
        }

        FrameVector<Index> indices;
        indices.resize(sliceCount * 6);

        for (int i = 0; i < sliceCount - 1; ++i)
        {
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
        v.position = { 0.0f, -1.0f, 0.0f, 1.0f };
        v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        v.uv = { 0.0f, 0.0f };
        vertices.push_back(v); // index: sliceCount * 2

        for (int i = 0; i < sliceCount - 1; i++)
        {
            indices.push_back(sliceCount * 2);
            indices.push_back(i * 2);
            indices.push_back((i + 1) * 2);
        }
        indices.push_back(sliceCount * 2);
        indices.push_back((sliceCount - 1) * 2);
        indices.push_back(0);

        // Top
        v.position = { 0.0f, 1.0f, 0.0f, 1.0f };
        v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        v.uv = { 0.0f, 0.0f };
        vertices.push_back(v); // index: sliceCount * 2 + 1

        for (int i = 0; i < sliceCount - 1; i++)
        {
            indices.push_back(sliceCount * 2 + 1);
            indices.push_back((i + 1) * 2 + 1);
            indices.push_back(i * 2 + 1);
        }
        indices.push_back(sliceCount * 2 + 1);
        indices.push_back(1);
        indices.push_back((sliceCount - 1) * 2 + 1);

        SetNormalVector(vertices, indices);

        SubMesh subMeshes[] = {
            {
                .vertexOffset = 0,
                .indexOffset = 0,
                .numIndices = indices.size(),
                .materialIndex = 0,
                .debugName = "Main"
            }
        };

        return std::make_shared<MeshData>(vertices, indices, subMeshes, "BaseCylinderMesh");
    }

    SharedPtr<MeshData> BaseMeshGenerator::GetCapsuleMeshData()
    {
        return nullptr;
    }

    SharedPtr<MeshData> BaseMeshGenerator::GetSphereMeshData()
    {
        constexpr int divisionNum = 3;

        // Create icosahedron
        constexpr float x = 0.525731f;
        constexpr float z = 0.850651f;

        FrameVector<Vertex> vertices;
        Vertex v;
        v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        v.uv = { 0.0f, 0.0f };

        v.position = { -x, 0.0f, z, 1.0f };
        vertices.push_back(v);
        v.position = { x, 0.0f, z, 1.0f };
        vertices.push_back(v);
        v.position = { -x, 0.0f, -z, 1.0f };
        vertices.push_back(v);
        v.position = { x, 0.0f, -z, 1.0f };
        vertices.push_back(v);
        v.position = { 0.0f, z, x, 1.0f };
        vertices.push_back(v);
        v.position = { 0.0f, z, -x, 1.0f };
        vertices.push_back(v);
        v.position = { 0.0f, -z, x, 1.0f };
        vertices.push_back(v);
        v.position = { 0.0f, -z, -x, 1.0f };
        vertices.push_back(v);
        v.position = { z, x, 0.0f, 1.0f };
        vertices.push_back(v);
        v.position = { -z, x, 0.0f, 1.0f };
        vertices.push_back(v);
        v.position = { z, -x, 0.0f, 1.0f };
        vertices.push_back(v);
        v.position = { -z, -x, 0.0f, 1.0f };
        vertices.push_back(v);

        FrameVector<Index> indices = {
            1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
            1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
            3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
            10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
        };

        // Divide vertices from 1 to 4
        for (int i = 0; i < divisionNum; ++i)
        {
            SubDivide(vertices, indices);
        }

        // Project onto sphere
        for (Vertex& vertex : vertices)
        {
            Vector3 v;
            v = vertex.position;
            v.Normalize();
            v *= 0.5f;

            Float3 vF = v.GetFloat3();

            vertex.position = { vF.x, vF.y, vF.z, 1.0f };
            vertex.normal = { vF.x, vF.y, vF.z, 0 };
            vertex.normal.Normalize();
        }

        //SetNormalVector(meshPtr);

        SubMesh subMeshes[] = {
            {
                .vertexOffset = 0,
                .indexOffset = 0,
                .numIndices = indices.size(),
                .materialIndex = 0,
                .debugName = "Main"
            }
        };

        return std::make_shared<MeshData>(vertices, indices, subMeshes, "BaseSphereMesh");
    }

    SharedPtr<MeshData> BaseMeshGenerator::GetPlaneMeshData()
    {
        FrameVector<Vertex> vertices;
        Vertex v;
        v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        v.uv = { 0.0f, 0.0f };

        v.position = { -5.0f, 0.0f, -5.0f, 1.0f };
        vertices.push_back(v);
        v.position = { -5.0f, 0.0f, 5.0f, 1.0f };
        vertices.push_back(v);
        v.position = { 5.0f, 0.0f, 5.0f, 1.0f };
        vertices.push_back(v);
        v.position = { 5.0f, 0.0f, -5.0f, 1.0f };
        vertices.push_back(v);

        FrameVector<Index> indices = {
            0,1,2,  0,2,3
        };

        SetNormalVector(vertices, indices);

        SubMesh subMeshes[] = {
            {
                .vertexOffset = 0,
                .indexOffset = 0,
                .numIndices = indices.size(),
                .materialIndex = 0,
                .debugName = "Main"
            }
        };

        return std::make_shared<MeshData>(vertices, indices, subMeshes, "BasePlaneMesh");
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
        for (int i = 0; i < numTriangles; ++i)
        {
            Vertex v0 = oldVertices[oldIndices[i * 3]];
            Vertex v1 = oldVertices[oldIndices[i * 3 + 1]];
            Vertex v2 = oldVertices[oldIndices[i * 3 + 2]];

            Vertex m0, m1, m2;
            m0.position = (v0.position + v1.position) / 2.0f;

            m1.position = (v1.position + v2.position) / 2.0f;

            m2.position = (v0.position + v2.position) / 2.0f;

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
        for (Vertex& v : vertices)
        {
            v.normal = { 0.0f, 0.0f, 0.0f, 1.0f };
        }

        int numTriangles = (int)indices.size() / 3;
        for (int i = 0; i < numTriangles; i++)
        {
            Index i0 = indices[i * 3];
            Index i1 = indices[i * 3 + 1];
            Index i2 = indices[i * 3 + 2];
            Vertex v0 = vertices[i0];
            Vertex v1 = vertices[i1];
            Vertex v2 = vertices[i2];

            Vector3 t0;
            t0 = v1.position - v0.position;
            Vector3 t1;
            t1 = v2.position - v0.position;

            Vector3 n = Vector3::Cross(t0, t1);

            vertices[i0].normal += n;
            vertices[i1].normal += n;
            vertices[i2].normal += n;
        }

        for (Vertex& v : vertices)
        {
            v.normal.Normalize();
        }
    }
} // namespace cube
