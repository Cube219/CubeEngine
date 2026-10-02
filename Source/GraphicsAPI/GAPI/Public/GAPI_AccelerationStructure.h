#pragma once

#include "GAPIHeader.h"

#include "Matrix.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;

        enum class ASBuildPreferType
        {
            Default,
            Performance,
            BuildTime,
        };

        struct BLASCreateInfo
        {
            SharedPtr<Buffer> vertexBuffer;
            ElementFormat vertexFormat = ElementFormat::RGBA32_Float;
            Uint64 vertexStride;

            SharedPtr<Buffer> indexBuffer = nullptr;

            struct GeometryInfo
            {
                Uint64 vertexOffset = 0;
                Uint64 vertexCount;
                Uint64 indexOffset = 0;
                Uint64 indexCount;

                StringView debugName;
            };
            ConstArrayView<GeometryInfo> geometryInfos;

            bool isOpaque = true;
            bool allowUpdate = false;
            ASBuildPreferType buildPreferType = ASBuildPreferType::Default;

            StringView debugName;
        };

        class BLAS
        {
        public:
            BLAS(const BLASCreateInfo& createInfo)
                : mAllowUpdate(createInfo.allowUpdate)
                , mDebugName(createInfo.debugName)
            {}
            virtual ~BLAS() = default;

            bool IsAllowUpdate() const { return mAllowUpdate; }

            Uint64 GetScratchBufferSize() const { return mScratchBufferSize; }
            Uint64 GetUpdateScratchBufferSize() const { return mUpdateScratchBufferSize; }

            SharedPtr<Buffer> GetVertexBufferToBuild() const { return mVertexBufferToBuild; }
            SharedPtr<Buffer> GetIndexBufferToBuild() const { return mIndexBufferToBuild; }

            StringView GetDebugName() const { return mDebugName; }

        protected:
            bool mAllowUpdate;

            Uint64 mScratchBufferSize = 0; // Set in child class
            Uint64 mUpdateScratchBufferSize = 0; // Set in child class

            SharedPtr<Buffer> mVertexBufferToBuild;
            SharedPtr<Buffer> mIndexBufferToBuild;

            String mDebugName;
        };

        struct BLASInstance
        {
            SharedPtr<BLAS> blas;
            Matrix transform;
            Uint32 hitGroupIndex;
        };

        struct TLASCreateInfo
        {
            ConstArrayView<BLASInstance> instances;

            bool allowUpdate = false;
            ASBuildPreferType buildPreferType = ASBuildPreferType::Default;

            StringView debugName;
        };

        class TLAS
        {
        public:
            TLAS(const TLASCreateInfo& createInfo)
                : mAllowUpdate(createInfo.allowUpdate)
                , mDebugName(createInfo.debugName)
            {}
            virtual ~TLAS() = default;

            bool IsAllowUpdate() const { return mAllowUpdate; }

            Uint64 GetScratchBufferSize() const { return mScratchBufferSize; }
            Uint64 GetUpdateScratchBufferSize() const { return mUpdateScratchBufferSize; }
            Uint64 GetInstanceDescBufferSize() const { return mInstanceDescBufferSize; }
            
            virtual void UpdateInstances(ConstArrayView<BLASInstance> instances) = 0;

        protected:
            bool mAllowUpdate;

            Uint64 mScratchBufferSize = 0; // Set in child class
            Uint64 mUpdateScratchBufferSize = 0; // Set in child class
            Uint64 mInstanceDescBufferSize = 0; // Set in child class

            String mDebugName;
        };
    } // namespace gapi
} // namespace cube
