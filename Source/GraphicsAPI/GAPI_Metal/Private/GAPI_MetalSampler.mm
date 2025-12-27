#include "GAPI_MetalSampler.h"

#include "Checker.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"

namespace cube
{
    namespace gapi
    {
        MTLSamplerMinMagFilter ConvertToMTLSamplerMinMagFilter(SamplerFilterType filter)
        {
            switch (filter)
            {
            case SamplerFilterType::Point:
                return MTLSamplerMinMagFilterNearest;
            case SamplerFilterType::Linear:
                return MTLSamplerMinMagFilterLinear;
            default:
                NOT_IMPLEMENTED();
                break;
            }
            return MTLSamplerMinMagFilterNearest;
        }

        MTLSamplerMipFilter ConvertToMTLSamplerMipFilter(SamplerFilterType filter)
        {
            switch (filter)
            {
            case SamplerFilterType::Point:
                return MTLSamplerMipFilterNearest;
            case SamplerFilterType::Linear:
                return MTLSamplerMipFilterLinear;
            default:
                NOT_IMPLEMENTED();
                break;
            }
            return MTLSamplerMipFilterNearest;
        }

        MTLSamplerAddressMode ConvertToMTLSamplerAddressMode(SamplerAddressMode addressMode)
        {
            // Not support MTLSamplerAddressModeClampToZero
            switch (addressMode)
            {
            case SamplerAddressMode::Wrap:
                return MTLSamplerAddressModeRepeat;
            case SamplerAddressMode::Mirror:
                return MTLSamplerAddressModeMirrorRepeat;
            case SamplerAddressMode::Clamp:
                return MTLSamplerAddressModeClampToEdge;
            case SamplerAddressMode::Border:
                return MTLSamplerAddressModeClampToBorderColor;
            case SamplerAddressMode::MirrorOnce:
                return MTLSamplerAddressModeMirrorClampToEdge;
            default:
                NOT_IMPLEMENTED();
                break;
            }
            return MTLSamplerAddressModeClampToEdge;
        }

        MTLSamplerBorderColor ConvertToMTLSamplerBorderColor(const float borderColor[4])
        {
            if (borderColor[0] == 0.0f && borderColor[1] == 0.0f && borderColor[2] == 0.0f)
            {
                if (borderColor[3] == 0.0f)
                {
                    return MTLSamplerBorderColorTransparentBlack;
                }
                else if (borderColor[3] == 1.0f)
                {
                    return MTLSamplerBorderColorOpaqueBlack;
                }
            }
            else if (borderColor[0] == 1.0f && borderColor[1] == 1.0f && borderColor[2] == 1.0f && borderColor[3] == 1.0f)
            {
                return MTLSamplerBorderColorOpaqueWhite;
            }

            CUBE_LOG(Warning, Metal, "Metal only support transparent black, opaque black and opaque white in sampler border color. Use transparent black instead.");
            return MTLSamplerBorderColorTransparentBlack;
        }

        MetalSampler::MetalSampler(const SamplerCreateInfo& info, MetalDevice& device)
        { @autoreleasepool {
            MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];
            samplerDesc.minFilter = ConvertToMTLSamplerMinMagFilter(info.minFilter);
            samplerDesc.magFilter = ConvertToMTLSamplerMinMagFilter(info.magFilter);
            samplerDesc.mipFilter = ConvertToMTLSamplerMipFilter(info.mipFilter);
            samplerDesc.maxAnisotropy = std::max(info.maxAnisotropy, 1u);
            samplerDesc.sAddressMode = ConvertToMTLSamplerAddressMode(info.addressU);
            samplerDesc.tAddressMode = ConvertToMTLSamplerAddressMode(info.addressV);
            samplerDesc.rAddressMode = ConvertToMTLSamplerAddressMode(info.addressW);
            samplerDesc.borderColor = ConvertToMTLSamplerBorderColor(info.borderColor);
            samplerDesc.normalizedCoordinates = YES;
            samplerDesc.lodMinClamp = info.minLod;
            samplerDesc.lodMaxClamp = info.maxLod;
            samplerDesc.lodBias = info.mipLodBias;
            samplerDesc.supportArgumentBuffers = YES;
            samplerDesc.label = String_Convert<NSString*>(info.debugName);

            mSamplerState = [device.GetMTLDevice() newSamplerStateWithDescriptor:samplerDesc];
            CHECK(mSamplerState);

            mBindlessId = mSamplerState.gpuResourceID._impl;
        }}

        MetalSampler::~MetalSampler()
        {
            [mSamplerState release];
        }
    } // namespace gapi
} // namespace cube
