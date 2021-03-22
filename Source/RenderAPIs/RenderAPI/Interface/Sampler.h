#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class FilterType
        {
            Point,
            Linear
        };

        enum class AddressMode
        {
            Wrap,
            BorderColor,
            Clamp,
            Mirror,
            MirrorOnce
        };

        struct SamplerCreateInfo
        {
            FilterType minFilter = FilterType::Linear;
            FilterType magFilter = FilterType::Linear;
            FilterType mipFilter = FilterType::Linear;

            AddressMode addressU = AddressMode::Wrap;
            AddressMode addressV = AddressMode::Wrap;
            AddressMode addressW = AddressMode::Wrap;

            Uint32 maxAnisotropy = 0;

            float mipLODBias = 0.0f;
            float minLOD = 0.0f;
            float maxLOD = 10000000.0f;

            float borderColor[4] = { 0, 0, 0, 0 };

            const char* debugName = "";
        };

        class Sampler
        {
        public:
            Sampler() = default;
            virtual ~Sampler() = default;
        };
    } // namespace rapi
} // namespace cube
