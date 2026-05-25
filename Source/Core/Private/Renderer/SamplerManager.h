#pragma once

#include "CoreHeader.h"

#include "GAPI_Sampler.h"
#include "Renderer/RenderTypes.h"

namespace cube
{
    class GAPI;

    class SamplerManager
    {
    public:
        SamplerManager() = default;
        ~SamplerManager() = default;

        void Initialize(GAPI* gapi);
        void Shutdown();

        // Note: Add SharedPtr<gapi::Sampler> version if needed
        Uint64 GetDefaultLinearSamplerId();
        BindlessSampler GetSampler(const gapi::SamplerCreateInfo& createInfo);

    private:
        gapi::Sampler* GetOrCreateSampler(const gapi::SamplerCreateInfo& createInfo);

        GAPI* mGAPI;

        HashMap<Uint64, SharedPtr<gapi::Sampler>> mCachedSamplers;

        gapi::SamplerCreateInfo mDefaultLinearSamplerCreateInfo;
    };
} // namespace cube
