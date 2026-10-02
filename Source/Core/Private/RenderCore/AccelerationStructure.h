#pragma once

#include "Resource.h"

#include "GAPI_AccelerationStructure.h"

namespace cube
{
    class BLASResource : public Resource
    {
    public:
        static SharedPtr<BLASResource> Create(const gapi::BLASCreateInfo& createInfo);

    public:
        BLASResource(const gapi::BLASCreateInfo& createInfo);
        virtual ~BLASResource() = default;

        SharedPtr<gapi::BLAS> GetGAPIBLAS() const { return mGAPIBLAS; }

        SharedPtr<gapi::Buffer> GetVertexBufferToBuild() const { return mVertexBufferToBuild; }
        SharedPtr<gapi::Buffer> GetIndexBufferToBuild() const { return mIndexBufferToBuild; }

        void UpdateLastBuildRenderingFrame(Uint64 renderingFrame) { mLastBuildRenderingFrame = renderingFrame; }
        Uint64 GetLastBuildRenderingFrame() const { return mLastBuildRenderingFrame; }

    private:
        SharedPtr<gapi::BLAS> mGAPIBLAS;

        SharedPtr<gapi::Buffer> mVertexBufferToBuild;
        SharedPtr<gapi::Buffer> mIndexBufferToBuild;

        Uint64 mLastBuildRenderingFrame = 0;
    };
} // namespace cube
