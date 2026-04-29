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
    } // namespace gapi
} // namespace cube
