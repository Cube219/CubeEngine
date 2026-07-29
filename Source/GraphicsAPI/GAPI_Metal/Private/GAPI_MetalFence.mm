#include "GAPI_MetalFence.h"

#include "Checker.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"

namespace cube
{
    namespace gapi
    {
        MetalFence::MetalFence(const FenceCreateInfo& info, MetalDevice& device)
            : Fence(info)
        {
            if (mAllowCPUAccess)
            {
                mEvent = [device.GetMTLDevice() newSharedEvent];
                ((id<MTLSharedEvent>)mEvent).signaledValue = 0;
            }
            else
            {
                mEvent = [device.GetMTLDevice() newEvent];
            }

            mEvent.label = String_Convert<NSString*>(info.debugName);
        }

        MetalFence::~MetalFence()
        {
            mEvent = nil;
        }

        void MetalFence::Wait(Uint64 fenceValue)
        {
            CHECK(mAllowCPUAccess);

            id<MTLSharedEvent> sharedEvent = (id<MTLSharedEvent>)mEvent;
            if (fenceValue > 0 && sharedEvent.signaledValue < fenceValue)
            {
                [sharedEvent waitUntilSignaledValue:fenceValue timeoutMS:100000000000];
            }
        }

        void MetalFence::Signal(Uint64 fenceValue)
        {
            CHECK(mAllowCPUAccess);
            ((id<MTLSharedEvent>)mEvent).signaledValue = fenceValue;
        }

        Uint64 MetalFence::GetCompletedValue()
        {
            CHECK(mAllowCPUAccess);
            return ((id<MTLSharedEvent>)mEvent).signaledValue;
        }
    } // namespace gapi
} // namespace cube
