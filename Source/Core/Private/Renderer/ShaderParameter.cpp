#include "ShaderParameter.h"

#include "GAPI.h"

namespace cube
{
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
        mBufferPools.resize(numGPUSync);

        mCurrentIndex = 0;
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

    void ShaderParametersManager::InitializeShaderParameters(ShaderParameters* parameters, Uint32 bufferSize, StringView debugName)
    {
        parameters->mGPUSyncIndex = mCurrentIndex;
        parameters->mPooledBuffer = AllocateBuffer(bufferSize, debugName);
        parameters->mBufferPointer = (Byte*)parameters->mPooledBuffer.buffer->Map();
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

        parameters.mBufferPointer = nullptr;
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
