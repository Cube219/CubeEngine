#pragma once

#include "../CoreHeader.h"

#include "../Thread/MutexLock.h"

namespace cube
{
    class CORE_EXPORT Resource
    {
    public:
        Resource() : mRefCount(0)
        {}
        virtual ~Resource() {}

    protected:
        friend class ResourceManager;
        template <typename T>
        friend class ResourcePointer;

        Atomic<Uint32> mRefCount;
    };

    template <typename T>
    class ResourcePointer
    {
    public:
        ResourcePointer() : mResource(nullptr)
        {}
        ~ResourcePointer()
        {
            if(mResource != nullptr) {
                mResource->mRefCount--;
            }
        }
        ResourcePointer(nullptr_t nul) : mResource(nullptr)
        {}
        ResourcePointer(const ResourcePointer& other)
        {
            mResource = other.mResource;
            if(mResource != nullptr) {
                mResource->mRefCount++;
            }
        }
        ResourcePointer& operator=(const ResourcePointer& rhs)
        {
            if(this == &rhs) return *this;

            if(mResource != nullptr) {
                mResource->mRefCount--;
            }

            mResource = rhs.mResource;
            if(mResource != nullptr) {
                mResource->mRefCount++;
            }

            return *this;
        }
        ResourcePointer(ResourcePointer&& other) noexcept
        {
            mResource = other.mResource;
            other.mResource = nullptr;
        }
        ResourcePointer& operator=(ResourcePointer&& rhs) noexcept
        {
            mResource = rhs.mResource;
            rhs.mResource = nullptr;

            return *this;
        }

        bool operator==(nullptr_t nul) const { return mResource == nullptr; }
        bool operator!=(nullptr_t nul) const { return mResource != nullptr; }

        template <typename T2>
        ResourcePointer<T2> Cast()
        {
            ResourcePointer<T2> resPtr((T2*)mResource);
            return resPtr;
        }

        void Release()
        {}

        T* operator->() const
        {
            return (T*)mResource;
        }
        T& operator*() const
        {
            return *(T*)mResource;
        }

    private:
        friend class ResourceManager;
        friend class BaseMeshGenerator;
        template <typename T2>
        friend class ResourcePointer;

        ResourcePointer(T* res)
        {
            mResource = res;
            mResource->mRefCount++;
        }

        Resource* mResource;
    };

    template <typename T>
    using RPtr = ResourcePointer<T>;
} // namespace cube
