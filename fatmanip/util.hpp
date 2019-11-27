#pragma once

#include <exception>
#include <iostream>
#include <iomanip>

#include "mio.hpp"

template <typename T>
T& place(mio::mmap_source & mmap, size_t offset)
{
    auto offset_mask = alignof(T) - 1;
    if (offset_mask & offset)
        throw std::exception{ "The offset doesn't work with T's alignment" }; // TODO: more detailed error message

    T* tmp = reinterpret_cast<T*>(&mmap[offset]);
    return *tmp;
}

inline std::ostream& restore(std::ostream& os)
{
    os << std::noshowbase << std::left << std::setfill('0') << std::dec << std::setw(0);
    return os;
}

struct _TempHexObject {
    int n;
};

inline _TempHexObject hx(int n) {
    return _TempHexObject{ n };
}

inline std::ostream& operator<<(std::ostream& os, _TempHexObject tho)
{
    os << std::showbase << std::internal << std::hex << std::setw(tho.n);
    return os;
}

