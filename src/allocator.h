#pragma once

#include <stdexcept>
#include <bitset>

namespace griha {

template <typename T, size_t ChunkN>
class allocator_arena {

    struct chunk {
        std::bitset<ChunkN> state;
        chunk* next;
        T data[ChunkN];
    };

public:
    template <typename U>
    struct rebind {
        using other = allocator_arena<U, ChunkN>;
    };

    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

public:
    ~allocator_arena() {
        for (auto p = head_; p != nullptr;) {
            auto t = p;
            p = p->next;
            free(t);
        }
    }

    T* allocate(size_type n) {
        if (n > ChunkN)
            throw std::bad_alloc(); // only sequentially allocation is supported

        if (n == 0)
            return nullptr;

        // find sequence of n free elements
        auto p = head_;
        for (; p != nullptr; p = p->next) {
            auto& state = p->state;
            for (size_t i = find_zero_bit(state), j = i + 1;
                 i < state.size();
                 i = find_zero_bit(state, j + 1), j = i + 1) {

                bool found;
                std::tie(j, found) = find_non_zero_bit_n(state, n - 1, j); // found if n is achieved to 0
                if (!found)
                    continue;
                set_bits(state, i, j); // set elements are busy
                return &p->data[i]; // and return pointer on first element
            }
        }
        // no suitable chunk, create new
        p = reinterpret_cast<chunk*>(malloc(sizeof(chunk)));
        if (p == nullptr)
            throw std::bad_alloc();
        
        memset(p, 0, sizeof(chunk));
        p->next = head_;
        head_ = p;

        // dow allocate in new chunk
        set_bits(head_->state, 0, n);
        return head_->data;
    }

    void deallocate(T* p, size_type n) {
        for (auto ch = head_; ch != nullptr; ch = ch->next) {
            if (ch->data > p || &ch->data[ChunkN - 1] < p)
                continue;
            size_t i = p - ch->data;
            if (i + n > ChunkN)
                throw std::invalid_argument("n should contain value as in corresponding call of allocate");
            set_bits(ch->state, i, i + n, false);
        }
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U> void destroy(U* p) { p->~U(); }
private:
    inline size_t find_zero_bit(const std::bitset<ChunkN>& state, size_t pos = 0ul) {
        for (; pos < state.size() && state[pos]; ++pos);
        return pos;
    }

    inline std::pair<size_t, bool>
    find_non_zero_bit_n(const std::bitset<ChunkN>& state, size_t n, size_t pos = 0ul) {
        for (; pos < state.size() && n != 0 && !state[pos]; ++pos, --n);
        return {pos, n == 0};
    }

    inline void set_bits(std::bitset<ChunkN>& state, size_t f, size_t l, bool value = true) {
        for (; f != l; ++f) state[f] = value;
    }

private:
    chunk* head_ = {nullptr};
};

} // namespace griha