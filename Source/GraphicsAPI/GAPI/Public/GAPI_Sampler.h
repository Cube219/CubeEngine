#pragma once

#include "GAPIHeader.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        enum class SamplerFilterType
        {
            Point,
            Linear
        };

        enum class SamplerAddressMode
        {
            Warp,
            Mirror,
            Clamp,
            Border,
            MirrorOnce
        };

        struct SamplerCreateInfo
        {
            // TODO: comparison filter?
            SamplerFilterType minFilter;
            SamplerFilterType magFilter;
            SamplerFilterType mipFilter;

            SamplerAddressMode addressU;
            SamplerAddressMode addressV;
            SamplerAddressMode addressW;

            bool useAnisotropy;
            Uint32 maxAnisotropy;

            float mipLodBias;
            float minLod;
            float maxLod;

            float borderColor[4];

            const char* debugName = "Unknown";

            Uint64 GetHashValue() const
            {
                Uint64 h = 0;
                auto hash_combine = [&h](auto&& v)
                {
                    h ^= std::hash<std::decay_t<decltype(v)>>{}(v) + 0x9e3779b9 + (h << 6) + (h >> 2);
                };

                hash_combine(minFilter);
                hash_combine(magFilter);
                hash_combine(mipFilter);
                hash_combine(addressU);
                hash_combine(addressV);
                hash_combine(addressW);
                hash_combine(useAnisotropy);
                hash_combine(maxAnisotropy);
                hash_combine(mipLodBias);
                hash_combine(minLod);
                hash_combine(maxLod);
                for (int i = 0; i < 4; ++i)
                {
                    hash_combine(borderColor[i]);
                }
                // Ignore debugName
                return h;
            }
        };

        class Sampler
        {
        public:
            Sampler() :
                mBindlessIndex(-1)
            {}
            virtual ~Sampler() = default;

            int GetBindlessIndex() const { return mBindlessIndex; }

        protected:
            int mBindlessIndex;
        };
    } // namespace gapi
} // namespace cube
