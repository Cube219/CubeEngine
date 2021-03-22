#pragma once

#include "CoreHeader.h"

#include "Assertion.h"

namespace cube
{
    template <typename T>
    class CORE_EXPORT Handler
    {
    public:
        Handler() :
            mTable(nullptr),
            mTableIndex(Uint64InvalidValue),
            mID(Uint64InvalidValue)
        {}

        Handler(HandlerTable* table, Uint64 tableIndex, Uint64 id) :
            mTable(table),
            mTableIndex(tableIndex),
            mID(id)
        {}

        template <typename T2>
        Handler(const Handler<T2>& other) :
            mTable(other.mTable),
            mTableIndex(other.mTableIndex),
            mID(other.mID)
        {}
        template <typename T2>
        Handler& operator=(const Handler<T2>& other)
        {
            mTable = other.mTable;
            mTableIndex = other.mTableIndex;
            mID = other.mID;

            return *this;
        }

        T* operator->() const
        {
            T* data = GetData();

            CHECK(data != nullptr, "Handler<{0}> does not have the data.", typeid(T).name());
            return &*(data);
        }
        T& operator*() const
        {
            T* data = GetData();

            CHECK(data != nullptr, "Handler<{0}> does not have the data.", typeid(T).name());
            return *(data);
        }

        bool operator==(const Handler& rhs)
        {
            return (mTable == rhs.mTable) && (mTableIndex == rhs.mTableIndex) && (mID == rhs.mID);
        }

        bool IsDestroyed() const
        {
            return GetData() == nullptr;
        }

        bool IsNull() const
        {
            return mTable == nullptr || mTableIndex == Uint64InvalidValue || mID == Uint64InvalidValue;
        }

    protected:
        friend T;
        template <typename T2>
        friend class Handler;
        friend class HandlerTable;

        // TODO: type check?
        T* GetData() const
        {
            CHECK(!IsNull(), "Cannot access the data from null handler.");

            T* data = mTable->GetData<T>(mTableIndex);

            // Check if it is a stale object
            if(data->GetID() != mID)
                return nullptr;

            return data;
        }

        HandlerTable* mTable;
        Uint64 mTableIndex;
        Uint64 mID;
    };

    class CORE_EXPORT Handlable
    {
    public:
        Handlable() :
            mID(Uint64InvalidValue)
        {}
        virtual ~Handlable() {}

        // TODO: 구조 좀 더 효율적으로 수정(매크로 써서 무조건 구현해야 할 것 추가하거나, 없애거나)
        Handler<Handlable> GetHandler() const
        {
            ASSERTION_FAILED("You must overload GetHandler function in a child class.");
            return Handler<Handlable>();
        }

        Uint64 GetID() const { return mID; }
        void SetID(Uint64 id) { mID = id; }

    protected:
        friend class HandlerTable;

        template <typename T>
        void SetHandler(Handler<T>& handler)
        {
            mMyHandler = handler;
        }

        Uint64 mID;
        Handler<Handlable> mMyHandler;
    };

    class HandlerTable
    {
    public:
        HandlerTable(Uint64 initialSize) :
            mLastAllocatedIndex(initialSize - 1),
            mNextID(0)
        {
            mTable.resize(initialSize, nullptr);
        }
        ~HandlerTable() {}

        HandlerTable(const HandlerTable& other) = delete;
        HandlerTable& operator=(const HandlerTable& other) = delete;

        template <typename T>
        Handler<T> CreateNewHandler(T* data)
        {
            Uint64 index = FindFreeSlot();
            mTable[index] = data;
            mLastAllocatedIndex = index;

            data->SetID(mNextID);
            mNextID++;

            auto h = Handler<T>(this, index, data->GetID());
            data->SetHandler(h);

            return h;
        }

        template <typename T>
        T* ReleaseHandler(Handler<T>& handler)
        {
            CHECK(handler.IsNull() == false, "You cannot release the null handler.");
            CHECK(handler.mTable == this, "You try to release the handler from wrong handler table.");
            // Check if it is a stale object
            CHECK(handler.mID == handler->GetID(), "This handler was already released.");

            T* data = DCast(T*)(mTable[handler.mTableIndex]);
            mTable[handler.mTableIndex] = nullptr;

            return data;
        }

        void ReleaseAll()
        {
            mTable.clear();
        }

        template <typename T>
        T* GetData(Uint64 index) const { return DCast(T*)(mTable[index]); }

    private:
        Uint64 FindFreeSlot()
        {
            for(Uint64 i = mLastAllocatedIndex + 1; i < mTable.size(); i++) {
                if(mTable[i] == nullptr) {
                    return i;
                }
            }

            for(Uint64 i = 0; i < mLastAllocatedIndex; i++) {
                if(mTable[i] == nullptr) {
                    return i;
                }
            }

            Uint64 oldSize = mTable.size();
            mTable.resize(oldSize * 2);

            return oldSize;
        }

        Vector<Handlable*> mTable;
        Uint64 mLastAllocatedIndex;
        Uint64 mNextID;
    };
} // namespace cube
