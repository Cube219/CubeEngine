#pragma once

#include "../CoreHeader.h"

#include "Mesh.h"

#include "../Allocator/FrameAllocator.h"
#include "../Resource/Resource.h"

namespace cube
{
    class CORE_EXPORT BaseMeshGenerator
    {
    public:
        static RPtr<Mesh> GetBoxMesh();

        static RPtr<Mesh> GetCylinderMesh();

        static RPtr<Mesh> GetCapsuleMesh(); // TODO: 차후 구현

        static RPtr<Mesh> GetSphereMesh();

        static RPtr<Mesh> GetPlaneMesh();

    private:
        static void SubDivide(FrameVector<Vertex>& vertices, FrameVector<Index>& indices);

        static void SetNormalVector(FrameVector<Vertex>& vertices, FrameVector<Index>& indices);

        static RPtr<Mesh> RegisterToResourceManager(Mesh* mesh);

        static Uint32 nextTempID;
    };
} // namespace cube
