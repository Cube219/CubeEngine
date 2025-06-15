#pragma once

#include "CoreHeader.h"

#include "Allocator/FrameAllocator.h"
#include "Renderer/Mesh.h"

namespace cube
{
    class MeshData;

    class BaseMeshGenerator
    {
    public:
        static SharedPtr<MeshData> GetBoxMeshData();
        static SharedPtr<MeshData> GetCylinderMeshData();
        static SharedPtr<MeshData> GetCapsuleMeshData(); // TODO
        static SharedPtr<MeshData> GetSphereMeshData();
        static SharedPtr<MeshData> GetPlaneMeshData();

    private:
        static void SubDivide(FrameVector<Vertex>& vertices, FrameVector<Index>& indices);
        static void SetNormalVector(FrameVector<Vertex>& vertices, FrameVector<Index>& indices);
    };
} // namespace cube
