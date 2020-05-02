#pragma once

#include "Types.h"

namespace cube
{
    // Refer to bs::framefork
    template <typename Enum, typename BitsType = Uint32>
    class Flags
    {
    public:
        Flags() :
            mBits(0)
        {}

        Flags(const Flags& other) :
            mBits(other.mBits)
        {}
        Flags& operator=(const Flags& rhs)
        {
            mBits = rhs.mBits;

            return *this;
        }

        Flags(Enum bit) :
            mBits(SCast(BitsType)(bit))
        {}
        Flags& operator=(Enum bit)
        {
            mBits = SCast(BitsType)(bit);

            return *this;
        }

        explicit Flags(BitsType bits) :
            mBits(bits)
        {}
        Flags& operator=(BitsType bits)
        {
            mBits = bits;
        }

        bool IsSet(Enum bit) const
        {
            return (mBits & SCast(BitsType)(bit)) != 0;
        }

        void Set(Enum bit)
        {
            mBits |= SCast(BitsType)(bit);
        }

        // Compare operations
        bool operator==(const Flags& rhs) const
        {
            return mBits == rhs.mBits;
        }
        bool operator==(Enum bit) const
        {
            return mBits == SCast(BitsType)(bit);
        }

        bool operator!=(const Flags& rhs) const
        {
            return mBits != rhs.mBits;
        }
        bool operator!=(Enum bit) const
        {
            return mBits != SCast(BitsType)(bit);
        }

        // OR(|) operations
        Flags& operator|=(const Flags& rhs)
        {
            mBits |= rhs.mBits;

            return *this;
        }
        Flags& operator|=(Enum bit)
        {
            mBits |= SCast(BitsType)(bit);

            return *this;
        }

        Flags operator|(const Flags& rhs) const
        {
            Flags res(*this);
            res |= rhs;

            return res;
        }
        Flags operator|(Enum bit) const
        {
            Flags res(*this);
            res |= bit;

            return res;
        }

        // AND(&) operations
        Flags& operator&=(const Flags& rhs)
        {
            mBits &= rhs.mBits;

            return *this;
        }
        Flags& operator&=(Enum bit)
        {
            mBits &= SCast(BitsType)(bit);

            return *this;
        }

        Flags operator&(const Flags& rhs) const
        {
            Flags res(*this);
            res |= rhs;

            return res;
        }
        Flags& operator&(Enum bit) const
        {
            Flags res(*this);
            res |= SCast(BitsType)(bit);

            return res;
        }

    private:
        BitsType mBits;
    };

#define FLAGS_OPERATOR(Enum) FLAGS_OPERATOR_EXT(Enum, Uint32)

#define FLAGS_OPERATOR_EXT(Enum, BitsType)                     \
    inline Flags<Enum, BitsType> operator|(Enum lhs, Enum rhs) \
    {                                                          \
        Flags<Enum, BitsType> res(lhs);                        \
        res |= rhs;                                            \
        return res;                                            \
    }                                                          \
    inline Flags<Enum, BitsType> operator&(Enum lhs, Enum rhs) \
    {                                                          \
        Flags<Enum, BitsType> res(lhs);                        \
        res &= rhs;                                            \
        return res;                                            \
    }

} // namespace cube
