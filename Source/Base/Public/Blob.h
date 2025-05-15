#pragma once

#include "Types.h"

namespace cube
{
    class BlobView
    {
    public:
        BlobView() :
            mSize(0),
            mData(nullptr)
        {}

        Uint64 GetSize() const { return mSize; }
        const void* GetData() const { return mData; }

    private:
        friend class Blob;

        Uint64 mSize;
        void* mData;
    };

    class Blob
    {
    public:
        Blob() :
            mSize(0),
            mData(nullptr)
        {}
        Blob(Uint64 size) :
            mSize(size)
        {
            mData = malloc(size);
        }
        Blob(void* data, Uint64 size) :
            mSize(size)
        {
            mData = malloc(size);
            memcpy(mData, data, mSize);
        }

        Blob(const Blob& other)
        {
            mSize = other.mSize;
            mData = malloc(mSize);
            memcpy(mData, other.mData, mSize);
        }
        Blob& operator=(const Blob& rhs)
        {
            if (this != &rhs)
            {
                Release();
                mSize = rhs.mSize;
                mData = malloc(mSize);
                memcpy(mData, rhs.mData, mSize);
            }
            return *this;
        }

        Blob(Blob&& other) noexcept
        {
            mSize = other.mSize;
            mData = other.mData;

            other.Release();
        }
        Blob& operator=(Blob&& rhs) noexcept
        {
            if (this != &rhs)
            {
                Release();
                mSize = rhs.mSize;
                mData = rhs.mData;
                rhs.Release();
            }
            return *this;
        }

        ~Blob()
        {
            Release();
        }

        void Release()
        {
            if (mData)
            {
                free(mData);
                mData = nullptr;

                mSize = 0;
            }
        }

        operator BlobView() const
        {
            BlobView res;
            res.mData = mData;
            res.mSize = mSize;

            return res;
        }

        Uint64 GetSize() const { return mSize; }
        void* GetData() const { return mData; }

    private:
        friend class BlobView;

        Uint64 mSize;
        void* mData;
    };
} // namespace cube
