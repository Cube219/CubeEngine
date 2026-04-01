#include "Renderer/ShaderParameter.h"

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
        mManager.FreeBuffer(*this);
    }

    void ShaderParameterListManager::Initialize(GAPI* gapi, Uint32 numGPUSync)
    {
        mGAPI = gapi;
        mShaderParameterHelper = &mGAPI->GetShaderParameterHelper();
        mCompatibleShaderParameterReflectionTypeMap = mShaderParameterHelper->GetCompatibleShaderParameterReflectionTypeMap();
        mBufferPools.resize(numGPUSync);

        mCurrentIndex = 0;

        ProcessDeferredInitializingParameterListInfos(*mShaderParameterHelper);
    }

    void ShaderParameterListManager::Shutdown()
    {
        for (ShaderParameterListBufferPool& pool : mBufferPools)
        {
            pool.Clear();
        }
        mBufferPools.clear();
    }

    void ShaderParameterListManager::MoveNextFrame()
    {
        mCurrentIndex = (mCurrentIndex + 1) % mBufferPools.size();

        ShaderParameterListBufferPool& pool = mBufferPools[mCurrentIndex];
        pool.CheckConsistency();

        for (Uint32 index : pool.freedBufferIndices)
        {
            pool.pooledBufferIndices.emplace((Uint32)(pool.buffers[index]->GetSize()), index);
        }
        pool.freedBufferIndices.clear();
    }

    bool ShaderParameterListManager::ValidateShader(const gapi::ShaderReflection& shaderReflection)
    {
        bool res = true;

        for (const gapi::ShaderParameterBlockReflection& blockReflection : shaderReflection.blocks)
        {
            const int index = GetShaderParameterListInfoIndex(blockReflection.typeName);
            if (index == -1)
            {
                CUBE_LOG(Error, ShaderParameter, "Undefined shader parameter list type! ({0})", blockReflection.typeName);
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
        // Constant buffer size must be 256 byte aligned in HLSL.
        Uint32 bufferSize = parameterListInfo.totalBufferSize;
        if ((bufferSize & 255) != 0)
        {
            bufferSize = (bufferSize + 255) & ~255;
        }

        parameterList->mGPUSyncIndex = mCurrentIndex;
        parameterList->mPooledBuffer = AllocateBuffer(bufferSize, parameterListInfo.name);
    }

    ShaderParameterListPooledBuffer ShaderParameterListManager::AllocateBuffer(Uint32 size, StringView debugName)
    {
        ShaderParameterListBufferPool& pool = mBufferPools[mCurrentIndex];

        if (auto it = pool.pooledBufferIndices.lower_bound(size); it != pool.pooledBufferIndices.end())
        {
            Uint32 index = it->second;
            SharedPtr<gapi::Buffer> buffer = pool.buffers[index];
            pool.pooledBufferIndices.erase(it);

            buffer->SetDebugName(debugName);
            return { .buffer = buffer, .poolIndex = index };
        }

        SharedPtr<gapi::Buffer> buffer = mGAPI->CreateBuffer({
            .type = gapi::BufferType::Constant,
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = size,
            .debugName = debugName
        });
        pool.buffers.push_back(buffer);

        return { .buffer = buffer, .poolIndex = static_cast<Uint32>(pool.buffers.size()) - 1 };
    }

    void ShaderParameterListManager::FreeBuffer(ShaderParameterList& parameterList)
    {
        if (mCurrentIndex != parameterList.mGPUSyncIndex)
        {
            CUBE_LOG(Warning, Renderer, "Mismatch GPU sync index at allocation and freeing in {0}. They should be the same GPU sync index.");
        }

        ShaderParameterListBufferPool& pool = mBufferPools[parameterList.mGPUSyncIndex];
        CHECK(parameterList.mPooledBuffer.buffer);
        CHECK(parameterList.mPooledBuffer.buffer == pool.buffers[parameterList.mPooledBuffer.poolIndex]);
        parameterList.mPooledBuffer.buffer->SetDebugName(CUBE_T("PooledShaderParameter"));
        pool.freedBufferIndices.push_back(parameterList.mPooledBuffer.poolIndex);

        parameterList.mPooledBuffer = { .buffer = nullptr, .poolIndex = 0 };
    }

    void ShaderParameterListManager::ShaderParameterListBufferPool::CheckConsistency()
    {
        CHECK(buffers.size() == freedBufferIndices.size() + pooledBufferIndices.size());

        Vector<bool> mark(buffers.size(), false);
        for (const auto& [_, index] : pooledBufferIndices)
        {
            CHECK_FORMAT(!mark[index], "Duplicated index! {0}", index);
            mark[index] = true;
        }
        for (Uint32 index : freedBufferIndices)
        {
            CHECK_FORMAT(!mark[index], "Duplicated index! {0}", index);
            mark[index] = true;
        }

        for (Uint32 i = 0; i < mark.size(); ++i)
        {
            CHECK_FORMAT(mark[i], "Not pooled index! {0}", i);
        }
    }
} // namespace cube
