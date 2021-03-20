#include "basic_flat_set.hpp"

#include "details/small_vector_bind.hpp"

namespace Ubpa {
    template<typename Key, std::size_t N = 16, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>>
    using small_flat_set = basic_flat_set<details::small_vector_bind<N, Allocator>::template Ttype,
        Key, Compare>;
}