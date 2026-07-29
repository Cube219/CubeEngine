#pragma once

#include "MetalHeader.h"

#include "GAPI_Fence.h"

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        class MetalFence : public Fence
        {
        public:
            MetalFence(const FenceCreateInfo& info, MetalDevice& device);
            virtual ~MetalFence();

            virtual void Wait(Uint64 fenceValue) override;
            virtual Uint64 GetCompletedValue() override;

            id<MTLSharedEvent> GetSharedEvent() const { return mSharedEvent; }

        private:
            id<MTLSharedEvent> mSharedEvent;
        };
    } // namespace gapi
} // namespace cube
