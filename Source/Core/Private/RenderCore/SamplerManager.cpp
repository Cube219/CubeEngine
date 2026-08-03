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
            .addressU = SamplerAddressMode::Wrap,
            .addressV = SamplerAddressMode::Wrap,
            .addressW = SamplerAddressMode::Wrap,
        };

        mDefaultLinearSamplerCreateInfo = defaultSamplerCreateInfo;
        mDefaultLinearSamplerCreateInfo.minFilter = SamplerFilterType::Linear;
        mDefaultLinearSamplerCreateInfo.magFilter = SamplerFilterType::Linear;
        mDefaultLinearSamplerCreateInfo.mipFilter = SamplerFilterType::Linear;
        mDefaultLinearSamplerCreateInfo.debugName = CUBE_T("DefaultLinearSampler");
    }

    void SamplerManager::Shutdown()
    {
        mCachedSamplers.clear();
    }

    Uint64 SamplerManager::GetDefaultLinearSamplerId()
    {
        return GetOrCreateSampler(mDefaultLinearSamplerCreateInfo)->GetBindlessId();
    }

    BindlessSampler SamplerManager::GetSampler(const gapi::SamplerCreateInfo& createInfo)
    {
        return BindlessSampler{
            .id = GetOrCreateSampler(createInfo)->GetBindlessId()
        };
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
