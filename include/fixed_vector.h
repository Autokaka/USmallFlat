#pragma once

#include <cassert>
#include <stdexcept>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 26495 )
#endif

namespace Ubpa {

    template <class T, std::size_t N>
    class fixed_vector {
    public:
        //////////////////
        // Member types //
        //////////////////

        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = T*;
        using const_pointer = const T*;
        // TODO : improve iterator
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        //////////////////////
        // Member functions //
        //////////////////////

        fixed_vector() noexcept : size_{ 0 } {}

        explicit fixed_vector(size_type count) : size_{ count } {
            assert(count <= N);
            for (value_type& elem : *this)
                new(&elem)value_type{};
        }

        fixed_vector(size_type count, const value_type& value) : size_{ count } {
            assert(count <= N);
            for (value_type& elem : *this)
                new(&elem)value_type{ value };
        }

        fixed_vector(const fixed_vector& other) : size_(other.size_) {
            if constexpr (std::is_trivially_copy_constructible_v<value_type>)
                std::memcpy(&storage_, &other.storage_, size_ * sizeof(value_type));
            else {
                pointer cursor = data();
                for (const value_type& elem : other)
                    new(cursor++)value_type{ elem };
            }
        }

        fixed_vector(fixed_vector&& other) noexcept : size_(other.size_) {
            if constexpr (std::is_trivially_move_constructible_v<value_type>)
                std::memcpy(&storage_, &other.storage_, size_ * sizeof(value_type));
            else {
                pointer cursor = data();
                for (value_type& elem : other)
                    new(cursor++)value_type{ std::move(elem) };
            }
            other.clear();
        }

        fixed_vector(std::initializer_list<value_type> initlist) : size_{ initlist.size() } {
            assert(initlist.size() <= N);
            if constexpr (std::is_trivially_move_constructible_v<value_type>)
                std::memcpy(&storage_, initlist.begin(), size_ * sizeof(value_type));
            else {
                pointer cursor = data();
                for (const value_type& elem : initlist)
                    new(cursor++)value_type{ elem };
            }
        }

        ~fixed_vector() {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (value_type& elem : *this)
                    elem.~value_type();
            }
        }

        fixed_vector& operator=(const fixed_vector& rhs) {
            if (this != &rhs) {
                if (size_ > rhs.size_) {
                    if constexpr (!std::is_trivially_destructible_v<T>) {
                        pointer last = data() + size_;
                        for (pointer cursor = data() + rhs.size_; cursor != last; ++cursor)
                            cursor->~value_type();
                    }
                    std::copy(rhs.data(), rhs.data() + rhs.size_, data());
                }
                else {
                    if constexpr (std::is_trivially_copy_assignable_v<value_type>) {
                        if constexpr (std::is_trivially_copy_constructible_v<value_type>)
                            std::memcpy(&storage_, &rhs.storage_, rhs.size_ * sizeof(value_type));
                        else { // copy assignment + copy ctor
                            const_pointer copy_ctor_cursor = rhs.data() + size_;
                            std::copy(rhs.data(), copy_ctor_cursor, data());

                            pointer last = data() + rhs.size_;
                            for (pointer cursor = data() + size_; cursor != last; ++cursor)
                                new(cursor) value_type{ *copy_ctor_cursor++ };
                        }
                    }
                    else {
                        const_pointer copy_ctor_cursor = rhs.data() + size_;
                        std::copy(rhs.data(), copy_ctor_cursor, data());

                        if constexpr (std::is_trivially_copy_constructible_v<value_type>)
                            std::memcpy(data() + size_, rhs.data() + size_, (rhs.size_ - size_) * sizeof(value_type));
                        else {
                            pointer last = data() + rhs.size_;
                            for (pointer cursor = data() + size_; cursor != last; ++cursor)
                                new(cursor)value_type{ *copy_ctor_cursor++ };
                        }
                    }
                }
                size_ = rhs.size_;
            }
            return *this;
        }

        fixed_vector& operator=(fixed_vector&& rhs) noexcept {
            if (this != &rhs) {
                if (size_ > rhs.size_) {
                    if constexpr (!std::is_trivially_destructible_v<T>) {
                        pointer last = data() + size_;
                        for (pointer cursor = data() + rhs.size_; cursor != last; ++cursor)
                            cursor->~value_type();
                    }
                    if constexpr (std::is_trivially_move_assignable_v<value_type>)
                        std::memcpy(&storage_, &rhs.storage_, rhs.size_ * sizeof(value_type));
                    else {
                        pointer cursor = data();
                        for (value_type& elem : rhs)
                            *cursor = std::move(elem);
                    }
                }
                else {
                    if constexpr (std::is_trivially_move_assignable_v<value_type>) {
                        if constexpr (std::is_trivially_move_constructible_v<value_type>)
                            std::memcpy(&storage_, &rhs.storage_, rhs.size_ * sizeof(value_type));
                        else { // move assignment + move ctor
                            std::memcpy(&storage_, &rhs.storage_, size_ * sizeof(value_type));

                            pointer last = data() + rhs.size_;
                            for (pointer cursor = data() + size_, rhs_cursor = rhs.data() + size_; cursor != last; ++cursor, ++rhs_cursor)
                                new(cursor) value_type{ std::move(*rhs_cursor) };
                        }
                    }
                    else {
                        pointer move_ctor_cursor = rhs.data() + size_;
                        std::move(rhs.data(), move_ctor_cursor, data());

                        if constexpr (std::is_trivially_move_constructible_v<value_type>)
                            std::memcpy(data() + size_, rhs.data() + size_, (rhs.size_ - size_) * sizeof(value_type));
                        else {
                            pointer last = data() + rhs.size_;
                            for (pointer cursor = data() + size_; cursor != last; ++cursor)
                                new(cursor)value_type{ std::move(*move_ctor_cursor++) };
                        }
                    }
                }
                size_ = rhs.size_;
                rhs.clear();
            }
            return *this;
        }

        fixed_vector& operator=(std::initializer_list<value_type> rhs) {
            assert(rhs.size() <= N);
            const std::size_t rhs_size = rhs.size();
            if (size_ > rhs_size) {
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    pointer last = data() + size_;
                    for (pointer cursor = data() + rhs_size; cursor != last; ++cursor)
                        cursor->~value_type();
                }
                if constexpr (std::is_trivially_copy_assignable_v<value_type>)
                    std::memcpy(&storage_, rhs.begin(), rhs_size * sizeof(value_type));
                else {
                    pointer cursor = data();
                    for (const value_type& elem : rhs)
                        *cursor = elem;
                }
            }
            else {
                if constexpr (std::is_trivially_copy_assignable_v<value_type>) {
                    if constexpr (std::is_trivially_copy_constructible_v<value_type>)
                        std::memcpy(&storage_, rhs.begin(), rhs_size * sizeof(value_type));
                    else { // copy assignment + copy ctor
                        std::memcpy(&storage_, rhs.begin(), size_ * sizeof(value_type));

                        pointer last = data() + rhs.size_;
                        for (pointer cursor = data() + size_, rhs_cursor = rhs.begin() + size_; cursor != last; ++cursor, ++rhs_cursor)
                            new(cursor) value_type{ *rhs_cursor };
                    }
                }
                else {
                    const_pointer copy_ctor_cursor = rhs.begin() + size_;
                    std::copy(rhs.data(), copy_ctor_cursor, data());

                    if constexpr (std::is_trivially_copy_constructible_v<value_type>)
                        std::memcpy(data() + size_, rhs.data() + size_, (rhs_size - size_) * sizeof(value_type));
                    else {
                        pointer last = data() + rhs_size;
                        for (pointer cursor = data() + size_; cursor != last; ++cursor)
                            new(cursor)value_type{ std::copy(*copy_ctor_cursor++) };
                    }
                }
            }
            size_ = rhs_size;
            return *this;
        }
        
        // TODO: assign

        //
        // Element access
        ///////////////////

        reference at(size_type pos) {
            if (pos >= size())
                throw_out_of_range();

            return *(data() + pos);
        }

        const_reference at(size_type pos) const {
            if (pos >= size())
                throw_out_of_range();

            return *(data() + pos);
        }

        reference operator[](size_type pos) noexcept {
            assert(pos < size());
            return *(data() + pos);
        }

        const_reference operator[](size_type pos) const noexcept {
            assert(pos < size());
            return *(data() + pos);
        }

        reference front() noexcept {
            assert(!empty());
            return *data();
        }

        const_reference front() const noexcept {
            assert(!empty());
            return *data();
        }

        reference back() noexcept {
            assert(!empty());
            return *(data() + size_ - 1);
        }

        const_reference back() const noexcept {
            assert(!empty());
            return *(data() + size_ - 1);
        }

        pointer data() noexcept { return reinterpret_cast<pointer>(&storage_); }
        const_pointer data() const noexcept { return reinterpret_cast<const_pointer>(&storage_); }

        //
        // Iterators
        //////////////

        iterator begin() noexcept { return data(); }
        const_iterator begin() const noexcept { return data(); }
        const_iterator cbegin() const noexcept { return begin(); }

        iterator end() noexcept { return data() + size_; }
        const_iterator end() const noexcept { return data() + size_; }
        const_iterator cend() const noexcept { return end(); }

        reverse_iterator rbegin() noexcept { return reverse_iterator{ end() }; }
        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{ end() }; }
        const_reverse_iterator crbegin() const noexcept { return rbegin(); }

        reverse_iterator rend() noexcept { return reverse_iterator{ begin() }; }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator{ begin() }; }
        const_reverse_iterator crend() const noexcept { return rend(); }

        //
        // Capacity
        /////////////

        bool empty() const noexcept { return size_ == 0; }

        size_type size() const noexcept { return size_; }

        size_type max_size() const noexcept { return N; }

        size_type capacity() const noexcept { return N; }

        //
        // Modifiers
        //////////////

        void clear() noexcept {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for(value_type& elem : *this)
                    elem.~value_type();
            }
            size_ = 0;
        }

        // TODO: insert

        // TODO: emplace

        // TODO: erase

        void push_back(const value_type& value) {
            assert(size() < max_size());
            new(data() + size_)value_type{ value };
            ++size_;
        }

        void push_back(T&& value) {
            assert(size() < max_size());
            new(data() + size_)value_type{ std::move(value) };
            ++size_;
        }

        template<typename... Args>
        reference emplace_back(Args&&... args) {
            assert(size() < max_size());
            pointer addr = data() + size_;
            new(addr)value_type{ std::forward<Args>(args)... };
            ++size_;
            return *addr;
        }

        void pop_back() {
            assert(!empty());
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                back().~value_type();
            --size_;
        }

        void resize(size_type count) {
            assert(count < max_size());

            if (count > size_) {
                pointer last = data() + count;
                for (pointer cursor = data() + size_; cursor != last; ++cursor)
                    new(cursor) value_type{};
            }
            else {
                if constexpr (!std::is_trivially_destructible_v<value_type>) {
                    pointer last = data() + size_;
                    for (pointer cursor = data() + count; cursor != last; ++cursor)
                        cursor->~value_type();
                }
            }

            size_ = count;
        }

        void resize(size_type count, const T& value) {
            assert(count < max_size());

            if (count > size_) {
                pointer last = data() + count;
                for (pointer cursor = data() + size_; cursor != last; ++cursor)
                    new(cursor) value_type{value};
            }
            else {
                if constexpr (!std::is_trivially_destructible_v<value_type>) {
                    pointer last = data() + size_;
                    for (pointer cursor = data() + count; cursor != last; ++cursor)
                        cursor->~value_type();
                }
            }

            size_ = count;
        }

    private:
        [[noreturn]] void throw_out_of_range() const { throw std::out_of_range("invalid fixed_vector subscript"); }

        std::aligned_storage_t<sizeof(T)* N, alignof(T)> storage_;
        size_type size_;
    };

    template<typename T, std::size_t N>
    bool operator==(const fixed_vector<T, N>& lhs, const fixed_vector<T, N>& rhs) {
        return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template<typename T, std::size_t N>
    bool operator<(const fixed_vector<T, N>& lhs, const fixed_vector<T, N>& rhs) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<typename T, std::size_t N>
    bool operator!=(const fixed_vector<T, N>& lhs, const fixed_vector<T, N>& rhs) {
        return !(lhs == rhs);
    }

    template<typename T, std::size_t N>
    bool operator>(const fixed_vector<T, N>& lhs, const fixed_vector<T, N>& rhs) {
        return rhs < lhs;
    }

    template<typename T, std::size_t N>
    bool operator<=(const fixed_vector<T, N>& lhs, const fixed_vector<T, N>& rhs) {
        return !(rhs < lhs);
    }

    template<typename T, std::size_t N>
    bool operator>=(const fixed_vector<T, N>& lhs, const fixed_vector<T, N>& rhs) {
        return !(lhs < rhs);
    }
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif