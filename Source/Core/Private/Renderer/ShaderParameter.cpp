#include "Renderer/ShaderParameter.h"

#include "Allocator/FrameAllocator.h"
#include "GAPI.h"

namespace cube
{
    ShaderParametersManager::DeferredInitializingParameterInfos ShaderParametersManager::mDeferredInitializingParametersInfos[ShaderParametersManager::MAX_NUM_DEFERRED_INIT];
    int ShaderParametersManager::mDeferredInitializingParametersInfosIndex = 0;
    bool ShaderParametersManager::mIsDeferredInitOverflow = false;

    Vector<ShaderParametersInfo> ShaderParametersManager::mShaderParametersInfos;
    Map<String, int> ShaderParametersManager::mShaderParametersTypeNameToIndexMap;

    void ShaderParametersManager::AddDeferredInitializingParameterInfos(const DeferredInitializingParameterInfos& initInfos)
    {
        mDeferredInitializingParametersInfos[mDeferredInitializingParametersInfosIndex] = initInfos;
        mDeferredInitializingParametersInfosIndex++;

        if (mDeferredInitializingParametersInfosIndex >= MAX_NUM_DEFERRED_INIT)
        {
            mIsDeferredInitOverflow = true;
            // To avoid accessing out-of-bound.
            mDeferredInitializingParametersInfosIndex = 0;
        }
    }

    void ShaderParametersManager::ProcessDeferredInitializingParametersInfos(const gapi::ShaderParameterHelper& shaderParemeterHelper)
    {
        CHECK_FORMAT(!mIsDeferredInitOverflow, "Deferred init overflow! Increase ShaderParametersManager::MAX_NUM_DEFERRED_INIT. (Current: {0})", MAX_NUM_DEFERRED_INIT);

        for (int i = 0; i < mDeferredInitializingParametersInfosIndex; ++i)
        {
            DeferredInitializingParameterInfos& initInfo = mDeferredInitializingParametersInfos[i];
            String typeName = String(initInfo.typeName);
            if (mShaderParametersTypeNameToIndexMap.find(typeName) != mShaderParametersTypeNameToIndexMap.end())
            {
                CUBE_LOG(Error, ShaderParameter, "Shader parameters '{0}' is already initialized.", initInfo.typeName);
                continue;
            }

            mShaderParametersTypeNameToIndexMap.insert({ typeName, static_cast<int>(mShaderParametersInfos.size()) });
            ShaderParametersInfo& paramsInfo = mShaderParametersInfos.emplace_back();
            initInfo.initFunction(shaderParemeterHelper, paramsInfo);
        }

        mDeferredInitializingParametersInfosIndex = 0;
    }

    int ShaderParametersManager::GetShaderParametersInfoIndex(StringView parametersTypeName)
    {
        auto findIt = mShaderParametersTypeNameToIndexMap.find(String(parametersTypeName));
        if (findIt == mShaderParametersTypeNameToIndexMap.end())
        {
            CUBE_LOG(Error, ShaderParameter, "Uninitialized shader parameters type! ({0})", parametersTypeName);
            CHECK(false);
        }

        return findIt->second;
    }

    ShaderParameters::ShaderParameters(ShaderParametersManager& manager) :
        mManager(manager)
    {
        // Other members will be initialized in ShaderParametersManagers::InitializeShaderParameters.
    }

    ShaderParameters::~ShaderParameters()
    {
        mManager.FreeBuffer(*this);
    }

    void ShaderParametersManager::Initialize(GAPI* gapi, Uint32 numGPUSync)
    {
        mGAPI = gapi;
        mShaderParameterHelper = &mGAPI->GetShaderParameterHelper();
        mCompatibleShaderParameterReflectionTypeMap = mShaderParameterHelper->GetCompatibleShaderParameterReflectionTypeMap();
        mBufferPools.resize(numGPUSync);

        mCurrentIndex = 0;

        ProcessDeferredInitializingParametersInfos(*mShaderParameterHelper);
    }

    void ShaderParametersManager::Shutdown()
    {
        for (ShaderParametersBufferPool& pool : mBufferPools)
        {
            pool.Clear();
        }
        mBufferPools.clear();
    }

    void ShaderParametersManager::MoveNextFrame()
    {
        mCurrentIndex = (mCurrentIndex + 1) % mBufferPools.size();

        ShaderParametersBufferPool& pool = mBufferPools[mCurrentIndex];
        pool.CheckConsistency();

        for (Uint32 index : pool.freedBufferIndices)
        {
            pool.pooledBufferIndices.emplace((Uint32)(pool.buffers[index]->GetSize()), index);
        }
        pool.freedBufferIndices.clear();
    }

    bool ShaderParametersManager::ValidateShaderParameters(const Vector<ShaderParameterInfo>& parameterInfos, const gapi::ShaderParameterBlockReflection& parameterBlockReflection)
    {
        FrameMap<FrameString, int> parameterNameToIndexMapInShaderCode;
        FrameVector<bool> checkedParameterInShaderCode(parameterBlockReflection.params.size(), false);
        for (int i = 0; i < parameterBlockReflection.params.size(); ++i)
        {
            const gapi::ShaderParameterReflection& paramReflection = parameterBlockReflection.params[i];
            parameterNameToIndexMapInShaderCode.insert({ FrameString(paramReflection.name.begin(), paramReflection.name.end()), i });
        }

        bool res = true;

        for (const ShaderParameterInfo& parameterInfo : parameterInfos)
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

    void ShaderParametersManager::AllocateShaderParameters(ShaderParameters* parameters, const ShaderParametersInfo& parametersInfo)
    {
        // Constant buffer size must be 256 byte aligned in HLSL.
        Uint32 bufferSize = parametersInfo.totalBufferSize;
        if ((bufferSize & 255) != 0)
        {
            bufferSize = (bufferSize + 255) & ~255;
        }

        parameters->mGPUSyncIndex = mCurrentIndex;
        parameters->mPooledBuffer = AllocateBuffer(bufferSize, parametersInfo.name);
    }

    ShaderParametersPooledBuffer ShaderParametersManager::AllocateBuffer(Uint32 size, StringView debugName)
    {
        ShaderParametersBufferPool& pool = mBufferPools[mCurrentIndex];

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

    void ShaderParametersManager::FreeBuffer(ShaderParameters& parameters)
    {
        if (mCurrentIndex != parameters.mGPUSyncIndex)
        {
            CUBE_LOG(Warning, Renderer, "Mismatch GPU sync index at allocation and freeing in {0}. They should be the same GPU sync index.");
        }

        ShaderParametersBufferPool& pool = mBufferPools[parameters.mGPUSyncIndex];
        CHECK(parameters.mPooledBuffer.buffer);
        CHECK(parameters.mPooledBuffer.buffer == pool.buffers[parameters.mPooledBuffer.poolIndex]);
        parameters.mPooledBuffer.buffer->SetDebugName(CUBE_T("PooledShaderParameter")); 
        pool.freedBufferIndices.push_back(parameters.mPooledBuffer.poolIndex);

        parameters.mPooledBuffer = { .buffer = nullptr, .poolIndex = 0 };
    }

    void ShaderParametersManager::ShaderParametersBufferPool::CheckConsistency()
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
