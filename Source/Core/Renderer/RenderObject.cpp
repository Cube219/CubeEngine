#include "RenderObject.h"

namespace cube
{
    void RenderObject::OnCreate()
    {
        mRTObject = CreateRTObject();

        // TODO: lambda말고 rt::RenderObject의 함수로 실행
        RenderingThread::QueueTask([this]() {
            mRTObject->OnCreate();
        });
    }

    void RenderObject::OnDestroy()
    {
        // TODO: lambda말고 rt::RenderObject의 함수로 실행
        RenderingThread::QueueTask([ro = mRTObject]() {
            ro->OnDestroy();
        });

        mRTObject = nullptr;
    }
} // namespace cube
