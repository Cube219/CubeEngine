#include "GAPI_Resource.h"

#include "GAPI_Buffer.h"
#include "GAPI_Texture.h"

namespace cube
{
    namespace gapi
    {
        SubresourceRange SubresourceRangeInput::Clamp(const Texture* texture) const
        {
            return {
                .firstMipLevel = firstMipLevel,
                .mipLevels = mipLevels == SubresourceRangeInput::AllRange ? texture->GetMipLevels() - firstMipLevel : mipLevels,
                .firstSliceIndex = firstSliceIndex,
                .sliceSize = sliceSize == SubresourceRangeInput::AllRange ? texture->GetNumSlices() - firstSliceIndex : sliceSize
            };
        }

        SubresourceRange SubresourceRangeInput::Clamp(const TextureInfo& textureInfo) const
        {
            Uint32 numSlices = textureInfo.arraySize;
            if (textureInfo.type == TextureType::TextureCube || textureInfo.type == TextureType::TextureCubeArray)
            {
                numSlices *= 6;
            }
            return {
                .firstMipLevel = firstMipLevel,
                .mipLevels = mipLevels == SubresourceRangeInput::AllRange ? textureInfo.mipLevels - firstMipLevel : mipLevels,
                .firstSliceIndex = firstSliceIndex,
                .sliceSize = sliceSize == SubresourceRangeInput::AllRange ? numSlices  - firstSliceIndex : sliceSize
            };
        }
    } // namespace gapi
} // namespace cube
