#include "GAPI_Resource.h"

#include "GAPI_Buffer.h"
#include "GAPI_Texture.h"

namespace cube
{
    namespace gapi
    {

        SubresourceRange SubresourceRangeInput::Clamp(const Buffer* buffer) const
        {
            // Buffers has no subresource.
            return {
                .firstMipLevel = 0,
                .mipLevels = 1,
                .firstSliceIndex = 0,
                .sliceSize = 1
            };
        }

        SubresourceRange SubresourceRangeInput::Clamp(const Texture* texture) const
        {
            return {
                .firstMipLevel = firstMipLevel,
                .mipLevels = mipLevels < 0 ? texture->GetMipLevels() - firstMipLevel : static_cast<Uint32>(mipLevels),
                .firstSliceIndex = firstSliceIndex,
                .sliceSize = sliceSize < 0 ? texture->GetNumSlices() - firstSliceIndex : static_cast<Uint32>(sliceSize)
            };
        }
    } // namespace gapi
} // namespace cube
