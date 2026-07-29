#include "GAPI_MetalFence.h"

#include "MacOS/MacOSString.h"
#include "MetalDevice.h"

namespace cube
{
    namespace gapi
    {
        MetalFence::MetalFence(const FenceCreateInfo& info, MetalDevice& device)
        {
            mSharedEvent = [device.GetMTLDevice() newSharedEvent];
            mSharedEvent.label = String_Convert<NSString*>(info.debugName);
            mSharedEvent.signaledValue = 0;
        }

        MetalFence::~MetalFence()
        {
            mSharedEvent = nil;
        }

        void MetalFence::Wait(Uint64 fenceValue)
        {
            if (fenceValue > 0 && mSharedEvent.signaledValue < fenceValue)
            {
                [mSharedEvent waitUntilSignaledValue:fenceValue timeoutMS:100000000000];
            }
        }

        Uint64 MetalFence::GetCompletedValue()
        {
            return mSharedEvent.signaledValue;
        }
    } // namespace gapi
} // namespace cube
