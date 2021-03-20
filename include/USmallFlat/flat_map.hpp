#pragma once

#include "basic_flat_map.hpp"

#include "details/vector_bind.hpp"

namespace Ubpa {
    template<typename Key, typename T, typename Compare = std::less<Key>, template<typename>class TAllocator = std::allocator>
    using flat_map = basic_flat_map<details::Tvector_bind<TAllocator>::template Ttype,
        Key, T, Compare>;
}