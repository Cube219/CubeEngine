#include "SamplerManager.h"

#include "GAPI.h"

namespace cube
{
    void SamplerManager::Initialize(GAPI* gapi)
    {
        mGAPI = gapi;

        using namespace gapi;

        SamplerCreateInfo defaultSamplerCreateInfo = {
            .minFilter = SamplerFilterType::Point,
            .magFilter = SamplerFilterType::Point,
            .mipFilter = SamplerFilterType::Point,
            .addressU = SamplerAddressMode::Warp,
            .addressV = SamplerAddressMode::Warp,
            .addressW = SamplerAddressMode::Warp,
            .useAnisotropy = false,
            .maxAnisotropy = 0,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = std::numeric_limits<float>().max(),
            .borderColor = { 0.0f, 0.0f, 0.0f, 1.0f }
        };

        mDefaultLinearSamplerCreateInfo = defaultSamplerCreateInfo;
        mDefaultLinearSamplerCreateInfo.minFilter = SamplerFilterType::Linear;
        mDefaultLinearSamplerCreateInfo.magFilter = SamplerFilterType::Linear;
        mDefaultLinearSamplerCreateInfo.mipFilter = SamplerFilterType::Linear;
        mDefaultLinearSamplerCreateInfo.debugName = "DefaultLinearSampler";
    }

    void SamplerManager::Shutdown()
    {
        mCachedSamplers.clear();
    }

    int SamplerManager::GetDefaultLinearSamplerIndex()
    {
        return GetOrCreateSampler(mDefaultLinearSamplerCreateInfo)->GetBindlessIndex();
    }

    int SamplerManager::GetSamplerIndex(const gapi::SamplerCreateInfo& createInfo)
    {
        return GetOrCreateSampler(createInfo)->GetBindlessIndex();
    }

    gapi::Sampler* SamplerManager::GetOrCreateSampler(const gapi::SamplerCreateInfo& createInfo)
    {
        Uint64 hashValue = createInfo.GetHashValue();

        if (auto findIt = mCachedSamplers.find(hashValue); findIt != mCachedSamplers.end())
        {
            return findIt->second.get();
        }

        SharedPtr<gapi::Sampler> sampler = mGAPI->CreateSampler(createInfo);
        mCachedSamplers[hashValue] = sampler;

        return sampler.get();
    }
} // namespace cube
