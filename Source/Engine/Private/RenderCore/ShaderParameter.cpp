#include "RenderCore/ShaderParameter.h"

#include "Allocator/AllocatorUtility.h"
#include "Allocator/FrameAllocator.h"
#include "GAPI.h"

namespace cube
{
    ShaderParameterListManager::DeferredInitializingParameterListInfos ShaderParameterListManager::mDeferredInitializingParameterListInfos[ShaderParameterListManager::MAX_NUM_DEFERRED_INIT];
    int ShaderParameterListManager::mDeferredInitializingParameterListInfosIndex = 0;
    bool ShaderParameterListManager::mIsDeferredInitOverflow = false;

    Vector<ShaderParameterListInfo> ShaderParameterListManager::mShaderParameterListInfos;
    Map<String, int> ShaderParameterListManager::mShaderParameterListTypeNameToIndexMap;

    void ShaderParameterListManager::AddDeferredInitializingParameterListInfos(const DeferredInitializingParameterListInfos& initInfos)
    {
        mDeferredInitializingParameterListInfos[mDeferredInitializingParameterListInfosIndex] = initInfos;
        mDeferredInitializingParameterListInfosIndex++;

        if (mDeferredInitializingParameterListInfosIndex >= MAX_NUM_DEFERRED_INIT)
        {
            mIsDeferredInitOverflow = true;
            // To avoid accessing out-of-bound.
            mDeferredInitializingParameterListInfosIndex = 0;
        }
    }

    void ShaderParameterListManager::ProcessDeferredInitializingParameterListInfos(const gapi::ShaderParameterHelper& shaderParemeterHelper)
    {
        CHECK_FORMAT(!mIsDeferredInitOverflow, "Deferred init overflow! Increase ShaderParameterListManager::MAX_NUM_DEFERRED_INIT. (Current: {0})", MAX_NUM_DEFERRED_INIT);

        for (int i = 0; i < mDeferredInitializingParameterListInfosIndex; ++i)
        {
            DeferredInitializingParameterListInfos& initInfo = mDeferredInitializingParameterListInfos[i];
            String typeName = String(initInfo.typeName);
            if (mShaderParameterListTypeNameToIndexMap.find(typeName) != mShaderParameterListTypeNameToIndexMap.end())
            {
                CUBE_LOG(Error, ShaderParameter, "Shader parameter list '{0}' is already initialized.", initInfo.typeName);
                continue;
            }

            mShaderParameterListTypeNameToIndexMap.insert({ typeName, static_cast<int>(mShaderParameterListInfos.size()) });
            ShaderParameterListInfo& paramsInfo = mShaderParameterListInfos.emplace_back();
            initInfo.initFunction(shaderParemeterHelper, paramsInfo);
        }

        mDeferredInitializingParameterListInfosIndex = 0;
    }

    int ShaderParameterListManager::GetShaderParameterListInfoIndex(StringView parameterListTypeName)
    {
        auto findIt = mShaderParameterListTypeNameToIndexMap.find(String(parameterListTypeName));
        if (findIt == mShaderParameterListTypeNameToIndexMap.end())
        {
            return -1;
        }

        return findIt->second;
    }

    ShaderParameterList::ShaderParameterList(ShaderParameterListManager& manager) :
        mManager(manager)
    {
        // Other members will be initialized in ShaderParameterListManager::AllocateShaderParameterList.
    }

    ShaderParameterList::~ShaderParameterList()
    {
        mManager.Free(mAllocation);
    }

    void ShaderParameterListManager::Initialize(GAPI* gapi, Uint32 numGPUSync, Uint64 defaultConstantBufferSize)
    {
        mGAPI = gapi;
        mConstantBufferAlignment = mGAPI->GetInfo().constantBufferAlignment;
        mShaderParameterHelper = &mGAPI->GetShaderParameterHelper();
        mCompatibleShaderParameterReflectionTypeMap = mShaderParameterHelper->GetCompatibleShaderParameterReflectionTypeMap();
        mGPUBuffers.resize(numGPUSync);
        for (Uint32 i = 0; i < numGPUSync; ++i)
        {
            ShaderParameterListGPUBuffer& gpuBuffer = mGPUBuffers[i];

            gpuBuffer.stageBufferPtr = (Byte*)malloc(defaultConstantBufferSize);
            gpuBuffer.bufferSize = defaultConstantBufferSize;
            gpuBuffer.currentOffset = 0;

            gpuBuffer.constantBuffer = mGAPI->CreateBuffer(gapi::BufferCreateInfo{
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .bufferInfo = gapi::BufferInfo{
                    .type = gapi::BufferType::Constant,
                    .size = defaultConstantBufferSize,
                },
                .debugName = Format<FrameString>(CUBE_T("ShaderConstantBuffer_{0}"), i)
            });
            gpuBuffer.gpuMappedPtr = (Byte*)gpuBuffer.constantBuffer->Map();
            gpuBuffer.flushedOffset = 0;
        }

        mCurrentIndex = 0;

        ProcessDeferredInitializingParameterListInfos(*mShaderParameterHelper);
    }

    void ShaderParameterListManager::Shutdown()
    {
        for (ShaderParameterListGPUBuffer& gpuBuffer : mGPUBuffers)
        {
            gpuBuffer.constantBuffer->Unmap();
            free(gpuBuffer.stageBufferPtr);
        }
        mGPUBuffers.clear();
    }

    void ShaderParameterListManager::MoveNextFrame()
    {
        CHECK_FORMAT(mGPUBuffers[mCurrentIndex].currentOffset == mGPUBuffers[mCurrentIndex].flushedOffset, "Not all shader parameter data are flushed to GPU. Use FlushToGPUBuffer before submit command list.");

        mCurrentIndex = (mCurrentIndex + 1) % mGPUBuffers.size();

        ShaderParameterListGPUBuffer& gpuBuffer = mGPUBuffers[mCurrentIndex];
        gpuBuffer.currentOffset = 0;
        gpuBuffer.flushedOffset = 0;
    }

    void ShaderParameterListManager::FlushToGPUBuffer()
    {
        ShaderParameterListGPUBuffer& gpuBuffer = mGPUBuffers[mCurrentIndex];

        memcpy(gpuBuffer.gpuMappedPtr + gpuBuffer.flushedOffset, gpuBuffer.stageBufferPtr + gpuBuffer.flushedOffset, gpuBuffer.currentOffset - gpuBuffer.flushedOffset);
        gpuBuffer.flushedOffset = gpuBuffer.currentOffset;
    }

    bool ShaderParameterListManager::ValidateShader(const gapi::ShaderReflection& shaderReflection)
    {
        bool res = true;

        for (const gapi::ShaderParameterBlockReflection& blockReflection : shaderReflection.blocks)
        {
            const int index = GetShaderParameterListInfoIndex(blockReflection.typeName);
            if (index == -1)
            {
                CUBE_LOG(Error, ShaderParameter, "Unregistered shader parameter list type! ({0})", blockReflection.typeName);
                res = false;
            }
            else
            {
                if (!ValidateShaderParameterList(mShaderParameterListInfos[index], blockReflection))
                {
                    res = false;
                }
            }
        }

        return res;
    }

    bool ShaderParameterListManager::ValidateShaderParameterList(const ShaderParameterListInfo& parameterListInfo, const gapi::ShaderParameterBlockReflection& parameterBlockReflection)
    {
        FrameMap<FrameString, int> parameterNameToIndexMapInShaderCode;
        FrameVector<bool> checkedParameterInShaderCode(parameterBlockReflection.params.size(), false);
        for (int i = 0; i < parameterBlockReflection.params.size(); ++i)
        {
            const gapi::ShaderParameterReflection& paramReflection = parameterBlockReflection.params[i];
            parameterNameToIndexMapInShaderCode.insert({ FrameString(paramReflection.name.begin(), paramReflection.name.end()), i });
        }

        bool res = true;

        for (const ShaderParameterInfo& parameterInfo : parameterListInfo.parameterInfos)
        {
            FrameString name = String_Convert<FrameString>(parameterInfo.name);

            auto findIt = parameterNameToIndexMapInShaderCode.find(name);
            if (findIt == parameterNameToIndexMapInShaderCode.end())
            {
                CUBE_LOG(Error, ShaderParameter, "Cannot find shader parameter '{0}' in shader code!", name);
                res = false;
            }
            else
            {
                int index = findIt->second;
                const Vector<gapi::ShaderParameterReflection::Type>& compatibleTypes = mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(parameterInfo.type)];

                bool found = false;
                for (const gapi::ShaderParameterReflection::Type& compatibleType : compatibleTypes)
                {
                    if (compatibleType == parameterBlockReflection.params[index].type)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    CUBE_LOG(Error, ShaderParamter, "Incompatible shader parameter type between C++ and shader code! (Name: {0}::{1} / C++: {2} / shader code: {3})",
                        parameterBlockReflection.typeName, name, ToString(parameterInfo.type), ToString(parameterBlockReflection.params[index].type));
                    FrameString compatibleTypesStr;
                    for (const gapi::ShaderParameterReflection::Type& compatibleType : compatibleTypes)
                    {
                        if (compatibleTypesStr.empty())
                        {
                            compatibleTypesStr = ToString(compatibleType);
                        }
                        else
                        {
                            compatibleTypesStr = Format<FrameString>(CUBE_T("{0}, {1}"), compatibleTypesStr, ToString(compatibleType));
                        }
                    }
                    CUBE_LOG(Error, ShaderParamter, "    Compatible types in shader code: {0}", compatibleTypesStr);
                    res = false;
                }
                checkedParameterInShaderCode[index] = true;
            }
        }

        for (int i = 0; i < checkedParameterInShaderCode.size(); ++i)
        {
            if (!checkedParameterInShaderCode[i])
            {
                CUBE_LOG(Error, ShaderParameter, "Shader parameter '{0}' defined in shader code but cannot find in C++ code!", parameterBlockReflection.params[i].name);
                res = false;
            }
        }

        return res;
    }

    void ShaderParameterListManager::AllocateShaderParameterList(ShaderParameterList* parameterList, const ShaderParameterListInfo& parameterListInfo)
    {
        parameterList->mAllocation = Allocate(parameterListInfo.totalBufferSize, parameterListInfo.name);
    }

    ShaderParameterListBufferSubAllocation ShaderParameterListManager::Allocate(Uint32 size, StringView debugName)
    {
        ShaderParameterListGPUBuffer& gpuBuffer = mGPUBuffers[mCurrentIndex];

        Uint64 alignedOffset = Align(gpuBuffer.currentOffset, (Uint64)mConstantBufferAlignment);

        if (alignedOffset + size > gpuBuffer.bufferSize)
        {
            NO_ENTRY_FORMAT("Insufficient shader parameter buffer. Try to increase it.");
            return {};
        }

        void* stageBufferPtr = gpuBuffer.stageBufferPtr + alignedOffset;
        gpuBuffer.currentOffset = alignedOffset + size;

        return ShaderParameterListBufferSubAllocation{
            .gpuBuffer = gpuBuffer.constantBuffer,
            .offset = alignedOffset,
            .size = size,
            .stageSubAllocationPtr = stageBufferPtr,
        };
    }

    void ShaderParameterListManager::Free(ShaderParameterListBufferSubAllocation& allocation)
    {
        if (allocation.gpuBuffer != mGPUBuffers[mCurrentIndex].constantBuffer)
        {
            CUBE_LOG(Warning, Renderer, "Mismatch GPU buffer at allocation and freeing. The suballocation should be released in the same frame it was allocated.");
        }

        allocation = {};
    }
} // namespace cube
