#pragma once

#include "../CoreHeader.h"

#include "../Handler.h"
#include "RenderingThread.h"

namespace cube
{
    class CORE_EXPORT RenderObject : public Handlable
    {
    public:
        virtual ~RenderObject() = default;

        virtual void OnCreate();
        virtual void OnDestroy();

        virtual SPtr<rt::RenderObject> CreateRTObject() = 0;
        SPtr<rt::RenderObject> GetRTObject() const { return mRTObject; }

    protected:
        RenderObject() = default; // Only child class of RenderObject can create

        template <typename T>
        void SyncPrimaryData(T& src, T& dst)
        {
            RenderingThread::QueueSyncTask([&src, &dst]() {
                memcpy(&dst, &src, sizeof(decltype(src)));
            });
        }

        void QueueSyncTask(std::function<void()> syncTaskFunc)
        {
            RenderingThread::QueueSyncTask(syncTaskFunc);
        }

        SPtr<rt::RenderObject> mRTObject;
    };

    namespace rt
    {
        class CORE_EXPORT RenderObject
        {
        public:
            virtual ~RenderObject() = default;

            virtual void OnCreate() {}
            virtual void OnDestroy() {}

        protected:
            // friend class cube::RenderObject;

            RenderObject() = default; // Only RenderObject can create
        };
    } // namespace rt
} // namespace cube
