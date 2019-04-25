#pragma once

#include <type_traits>
#include <iterator>
#include <memory>
#include <limits>

namespace griha {

template <typename T, typename Alloc = std::allocator<T>>
class bidirectional_list {

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

private:
    struct node {
        T value;
        node* prev;
        node* next;
    };

    template <bool Const>
    class iterator_inner {
        template <typename, typename> friend class bidirectional_list;

        using list_type = std::conditional_t<!Const, bidirectional_list, const bidirectional_list>;

    public:
        using value_type = bidirectional_list::value_type;
        using reference = std::conditional_t<!Const, T&, const T&>;
        using pointer = std::conditional_t<!Const, T*, const T*>;
        using difference_type = ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        iterator_inner(const iterator_inner& src) : list_(src.list_), n_(src.n_) {}

        iterator_inner& operator=(const iterator_inner& rhs) {
            n_ = rhs.n_;
            return *this;
        }

        reference operator* () { return n_->value; }
        pointer operator-> () { return n_; }

        iterator_inner& operator++ () {
            if (n_ != nullptr)
                n_ = n_->next;
            return *this;
        }

        iterator_inner operator++ (int) {
            auto ret = *this;
            ++(*this);
            return ret;
        }

        iterator_inner& operator-- () {
            n_ = n_ != nullptr ? n_->prev : list_.tail_;
            return *this;
        }

        iterator_inner operator-- (int) {
            auto ret = *this;
            --(*this);
            return ret;
        }

        friend
        bool operator== (const iterator_inner<Const>& lhs, const iterator_inner<Const>& rhs) {
            return lhs.n_ == rhs.n_;
        }

    protected:
        explicit iterator_inner(list_type& list, node* n) : list_(list), n_(n) {}

    private:
        list_type& list_;
        node* n_;
    };

    template <bool Const>
    friend
    bool operator!= (const iterator_inner<Const>& lhs, const iterator_inner<Const>& rhs) {
        return !(lhs == rhs);
    }

    using alloc_type = typename Alloc::template rebind<node>::other;
    using alloc_traits = std::allocator_traits<alloc_type>;

public:
    class iterator : public iterator_inner<false> {
        template <typename, typename> friend class bidirectional_list;
        friend class const_iterator;

        using super = iterator_inner<false>;

    public:
        using typename super::value_type;
        using typename super::reference;
        using typename super::pointer;
        using typename super::difference_type;
        using typename super::iterator_category;

    public:
        iterator(const iterator& src) : super(src) {}

    private:
        iterator(bidirectional_list& list, node* n = nullptr) : super(list, n) {}
    };

    class const_iterator : public iterator_inner<true> {
        template <typename, typename> friend class bidirectional_list;

        using super = iterator_inner<true>;

    public:
        using typename super::value_type;
        using typename super::reference;
        using typename super::pointer;
        using typename super::difference_type;
        using typename super::iterator_category;

    public:
        const_iterator(const const_iterator& src) : super(src) {}
        const_iterator(const iterator& src) : super(src.list_, src.n_) {}

    private:
        const_iterator(const bidirectional_list& list, node* n = nullptr) : super(list, n) {}
    };

    using pointer = typename std::allocator_traits<Alloc>::pointer;
    using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
    using difference_type = ptrdiff_t;
    using size_type = size_t;

public:
    bidirectional_list() {}
    ~bidirectional_list() {
        for (; head_ != nullptr; head_ = head_->next) {
            alloc_traits::destroy(alloc_, reinterpret_cast<pointer>(head_));
            alloc_traits::deallocate(alloc_, head_, 1ul);
        }
        tail_ = nullptr;
    }

    bidirectional_list(const bidirectional_list& src) {
        if (src.empty())
            return;

        head_ = alloc_traits::allocate(alloc_, src.size());
        for (auto n = head_, n_src = src.head_;
             n_src != nullptr;
             ++n, n_src = n_src->next) {
                 
            alloc_traits::construct(alloc_, reinterpret_cast<pointer>(n), n_src->value);
            n->prev = tail_;
            n->next = nullptr;
            if (n->prev != nullptr)
                n->prev->next = n;
            tail_ = n;
        }
    }

    bidirectional_list(bidirectional_list&& src) {
        swap(src);
    }

    bidirectional_list& operator= (const bidirectional_list& rhs) {
        bidirectional_list t(rhs);
        swap(t);
        return *this;
    }

    bidirectional_list& operator= (bidirectional_list&& rhs) {
        swap(rhs);
        return *this;
    }

    iterator begin() { return iterator(*this, head_); }
    const_iterator begin() const { return const_iterator(*this, head_); }
    const_iterator cbegin() const { return const_iterator(*this, head_); }

    iterator end() { return iterator(*this, nullptr); }
    const_iterator end() const { return const_iterator(*this, nullptr); }
    const_iterator cend() const { return const_iterator(*this, nullptr); }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args... args) {
        node* nnode = alloc_traits::allocate(alloc_, 1ul);
        alloc_traits::construct(alloc_, reinterpret_cast<pointer>(nnode),
                                std::forward<Args>(args)...);

        auto& ref_from_r = pos.n_ != nullptr ? pos.n_->prev : tail_;
        auto& ref_from_l = ref_from_r != nullptr ? ref_from_r->next : head_;

        nnode->next = pos.n_;
        nnode->prev = ref_from_r;
        ref_from_r = ref_from_l = nnode;
        ++size_;

        return iterator(*this, nnode);
    }

    void erase(const_iterator pos) {
        if (pos.n_ == nullptr)
            return;

        auto& ref_from_r = pos.n_->next != nullptr ? pos.n_->next->prev : tail_;
        auto& ref_from_l = pos.n_->prev != nullptr ? pos.n_->prev->next : head_;
        
        ref_from_r = pos.n_->prev;
        ref_from_l = pos.n_->next;
        pos.n_->prev = pos.n_->next = nullptr;
        --size_;

        alloc_traits::destroy(alloc_, reinterpret_cast<pointer>(pos.n_));
        alloc_traits::deallocate(alloc_, pos.n_, 1ul);
    }

    size_type size() const { return size_; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }

    bool empty() const { return head_ == nullptr; }

    void swap(bidirectional_list& other) {
        std::swap(alloc_, other.alloc_);
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

private:
    alloc_type alloc_;
    node* head_ = {nullptr};
    node* tail_ = {nullptr};
    size_type size_ = {0};
};

template <typename T, typename Alloc>
void swap(bidirectional_list<T, Alloc>& lhs, bidirectional_list<T, Alloc>& rhs) {
    lhs.swap(rhs);
}

} // namespace griha