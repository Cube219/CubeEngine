#pragma once

#include "CoreHeader.h"

#include "Allocator/FrameAllocator.h"
#include "Renderer/Mesh.h"

namespace cube
{
    class MeshData;

    class MeshHelper
    {
    public:
        static SharedPtr<MeshData> GenerateBoxMeshData();
        static SharedPtr<MeshData> GenerateCylinderMeshData();
        static SharedPtr<MeshData> GenerateCapsuleMeshData(); // TODO
        static SharedPtr<MeshData> GenerateSphereMeshData();
        static SharedPtr<MeshData> GeneratePlaneMeshData();

        static void SetNormalVector(ArrayView<Vertex> inOutVertices, ArrayView<Index> inOutIndices);
        static void SetApproxTangentVector(ArrayView<Vertex> inOutVertices);

    private:
        static void SubDivide(ArrayView<Vertex> inOutVertices, ArrayView<Index> inOutIndices);
    };
} // namespace cube
