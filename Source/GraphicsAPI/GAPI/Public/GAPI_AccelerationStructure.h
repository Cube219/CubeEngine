#pragma once

#include "GAPIHeader.h"

#include "Matrix.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;

        enum class AccelerationStructureType
        {
            BLAS,
            TLAS,
        };

        enum class AccelerationStructureBuildPreferType
        {
            Default,
            Performance,
            BuildTime,
        };

        class AccelerationStructure
        {
        public:
            AccelerationStructure(AccelerationStructureType type, bool allowUpdate, StringView debugName)
                : mType(type)
                , mAllowUpdate(allowUpdate)
                , mDebugName(debugName)
            {}
            virtual ~AccelerationStructure() = default;

            AccelerationStructureType GetType() const { return mType; }
            bool IsUpdateAllowed() const { return mAllowUpdate; }

            StringView GetDebugName() const { return mDebugName; }

        private:
            AccelerationStructureType mType;
            bool mAllowUpdate;

            String mDebugName;
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
            AccelerationStructureBuildPreferType buildPreferType = AccelerationStructureBuildPreferType::Default;

            StringView debugName;
        };

        class BLAS : public AccelerationStructure
        {
        public:
            BLAS(const BLASCreateInfo& createInfo)
                : AccelerationStructure(AccelerationStructureType::BLAS, createInfo.allowUpdate, createInfo.debugName)
            {}
            virtual ~BLAS() = default;

            Uint64 GetScratchBufferSize() const { return mScratchBufferSize; }
            Uint64 GetUpdateScratchBufferSize() const { return mUpdateScratchBufferSize; }

        protected:
            Uint64 mScratchBufferSize = 0; // Set in child class
            Uint64 mUpdateScratchBufferSize = 0; // Set in child class
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
            AccelerationStructureBuildPreferType buildPreferType = AccelerationStructureBuildPreferType::Default;

            StringView debugName;
        };

        class TLAS : public AccelerationStructure
        {
        public:
            TLAS(const TLASCreateInfo& createInfo)
                : AccelerationStructure(AccelerationStructureType::TLAS, createInfo.allowUpdate, createInfo.debugName)
            {}
            virtual ~TLAS() = default;

            Uint64 GetScratchBufferSize() const { return mScratchBufferSize; }
            Uint64 GetUpdateScratchBufferSize() const { return mUpdateScratchBufferSize; }
            Uint64 GetInstanceDescBufferSize() const { return mInstanceDescBufferSize; }

            virtual void UpdateInstances(ConstArrayView<BLASInstance> instances) = 0;

        protected:
            Uint64 mScratchBufferSize = 0; // Set in child class
            Uint64 mUpdateScratchBufferSize = 0; // Set in child class
            Uint64 mInstanceDescBufferSize = 0; // Set in child class
        };
    } // namespace gapi
} // namespace cube
