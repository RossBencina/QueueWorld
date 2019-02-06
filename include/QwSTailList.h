/*
    Queue World is copyright (c) 2014-2018 Ross Bencina

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#ifndef INCLUDED_QWSTAILLIST_H
#define INCLUDED_QWSTAILLIST_H

#include <algorithm>
#include <cassert>
#ifdef NDEBUG
#include <cstdlib> // abort
#endif

#include "QwConfig.h"
#include "QwLinkTraits.h"

/*
    QwSTailList is a single-threaded singly linked list with support
    for O(1) push_back().

    Can be used as a FIFO queue stack (push and pop to front, push to back).

    The list is internally terminated with a nullptr next ptr.

    Constraints:
        - Don't call pop_front() on an empty list.

    Properties:
        - Many operations could be free functions (insert_after_node,
            remove_node_after, node_is_back, node is end, end).

        - O(1) swap contents of two lists

    In the simplest usage, Nodes must contain a links_ member that is an array
    of pointers to nodes. Alternatively, the client can specialize QwLinkTraits
    for a particular Node type.
    NEXT_LINK_INDEX specifies the element of this array that is used for the
    next ptr.

    struct ExampleNodeType{
        ExampleNodeType *links_[2]; // links isn't required to be at the top but it is required to be called links
        enum { EXAMPLE_LINK_INDEX_1, EXAMPLE_LINK_INDEX_2 }

        // ... other fields
    }

    typedef QwSTailList<ExampleNodeType*, ExampleNodeType::EXAMPLE_LINK_INDEX_1> ExampleListType;

    see forward_list for an interface reference:
    http://en.cppreference.com/w/cpp/container/forward_list
*/

template<typename NodePtrT, int NEXT_LINK_INDEX>
class QwSTailList{
    typedef QwLinkTraits<NodePtrT, NEXT_LINK_INDEX> nextlink;

public:
    typedef typename nextlink::node_type node_type;
    typedef typename nextlink::node_ptr_type node_ptr_type;
    typedef typename nextlink::const_node_ptr_type const_node_ptr_type;

private:

    node_ptr_type front_; // aka head. first link in list
    node_ptr_type back_; // last link in list

#if (QW_VALIDATE_NODE_LINKS == 1)
    void CHECK_NODE_IS_UNLINKED(const_node_ptr_type n) const
    {
#ifndef NDEBUG
        assert(nextlink::load(n) == nullptr); // (require unlinked)
        assert(n != front_);
        assert(n != back_);
        // Note: we can't check that the node is not referenced by some other list
#else
        if (!(nextlink::load(n) == nullptr)) { std::abort(); } // (require unlinked)
        if (!(n != front_)) { std::abort(); }
        if (!(n != back_)) { std::abort(); }
#endif
    }

    void CLEAR_NODE_LINKS_FOR_VALIDATION(node_ptr_type n) const
    {
        nextlink::store(n, nullptr);
    }
#else
    void CHECK_NODE_IS_UNLINKED(const_node_ptr_type) const {}
    void CLEAR_NODE_LINKS_FOR_VALIDATION(node_ptr_type) const {}
#endif

public:

    class iterator{
        node_ptr_type p_;
    public:
#if (QW_VALIDATE_NODE_LINKS == 1)
        iterator() : p_(nullptr) {}
#else
        iterator() {}
#endif

        explicit iterator(node_ptr_type p) : p_(p) {}

        iterator& operator++ ()     // prefix ++
        {
            p_ = nextlink::load(p_);
            return *this;
        }

        iterator operator++ (int)  // postfix ++
        {
            iterator result(*this);
            ++(*this);
            return result;
        }

        // list is a container of pointers so dereferencing the iterator gives a pointer
        node_ptr_type operator*() const { return p_; }
        const node_ptr_type* operator->() const { return &p_; }

        bool operator!=(const iterator& rhs) const { return rhs.p_ != p_; }
        bool operator==(const iterator& rhs) const { return rhs.p_ == p_; }
    };

    typedef iterator const_iterator;

    // TODO also provides const_iterator?

    QwSTailList() : front_(nullptr), back_(nullptr) {}

    void clear() {
#if (QW_VALIDATE_NODE_LINKS == 1)
        while (!empty()) pop_front();
#else
        // this doesn't mark nodes as unlinked
        front_ = nullptr;
        back_ = nullptr;
#endif
    }

    void swap(QwSTailList& other) {
        std::swap(front_, other.front_);
        std::swap(back_, other.back_);
    }
    //see also void swap(QwSTailList& a, QwSTailList &b);

    bool empty() const
    {
        return (front_ == nullptr);
    }

    bool size_is_1() const
    {
        return (front_ != nullptr && front_ == back_);
    }

    bool size_is_greater_than_1() const
    {
        return (front_ != nullptr && front_ != back_);
    }

    // front and back return nullptr when list is empty
    node_ptr_type front() { return front_; }
    const_node_ptr_type front() const { return front_; }

    node_ptr_type back() { return back_; }
    const_node_ptr_type back() const { return back_; }

    void push_front(node_ptr_type n)
    {
        CHECK_NODE_IS_UNLINKED(n);

        nextlink::store(n, front_); // this works even if front_ is nullptr when the list is empty.

        if (!front_)
            back_ = n;

        front_ = n;
    }

    node_ptr_type pop_front()
    {
        assert(!empty()); // this version of pop_front doesn't work on an empty list.
                          // caller should check is_empty() first.

        node_ptr_type result = front_;
        front_ = nextlink::load(front_);

        if (!front_)
            back_ = nullptr;

        CLEAR_NODE_LINKS_FOR_VALIDATION(result);
        return result;
    }

    void push_back(node_ptr_type n)
    {
        CHECK_NODE_IS_UNLINKED(n);

        nextlink::store(n, nullptr);

        if (empty()) {
            front_ = n;
        } else {
            nextlink::store(back_, n);
        }

        back_ = n;
    }

    void insert_after(node_ptr_type before, node_ptr_type n) // insert n after node before
    {
        assert(before != nullptr);
        assert(n != nullptr);
        CHECK_NODE_IS_UNLINKED(n);

        node_ptr_type after = nextlink::load(before);

        nextlink::store(n, after);
        nextlink::store(before, n);

        if (!after)
            back_ = n;
    }

    void insert_after(iterator before, node_ptr_type n) // insert n after node before.
                                                           // works even with before_begin() on an empty list.
    {
        insert_after(*before, n);
    }

    node_ptr_type remove_after(node_ptr_type before) // returns the removed node
    {
        assert(nextlink::load(before) != nullptr); // can't remove an item after the last item

        node_ptr_type result = nextlink::load(before);
        node_ptr_type next = nextlink::load(result);
        nextlink::store(before, next);

        if (!next) {
            if (front_ == nullptr) // (nextlink::load(before) aliases front when using before_begin())
                back_ = nullptr;
            else
                back_ = before;
        }

        CLEAR_NODE_LINKS_FOR_VALIDATION(result);
        return result;
    }

    void remove_after(iterator before)
    {
        remove_after(*before);
    }

    // erase_after returns an iterator to the item past the
    // item that was erased or end() if it was the last item
    iterator erase_after(iterator before)
    {
        assert(before != end());

        node_ptr_type before_node_ptr = *before;

        node_ptr_type erased_node_ptr = nextlink::load(before_node_ptr);
        node_ptr_type next_node_ptr = nextlink::load(erased_node_ptr);
        nextlink::store(before_node_ptr, next_node_ptr);

        if (!next_node_ptr) {
            if (front_ == nullptr) // (nextlink::load(before_node_ptr) aliases front when using before_begin())
                back_ = nullptr;
            else
                back_ = before_node_ptr;
        }

        CLEAR_NODE_LINKS_FOR_VALIDATION(erased_node_ptr);
        return iterator(nextlink::load(before_node_ptr));
    }

    // forward_list provides remove() and remove_if()

    iterator before_begin()
    {
        // pretend our front_ field is actually the next link field in a node struct
        // offset backwards from front_ then cast to a node ptr and wrap in an iterator
        // this is probably not strictly portable but it allows us to insert at the beginning.
        return iterator(reinterpret_cast<node_ptr_type>(reinterpret_cast<char*>(&front_) - nextlink::offsetof_link()));
    }

    iterator begin() const { return iterator(front_); }

    const iterator end() const { return iterator(nullptr); }

    // forward_list also provides const iterator and const iterator accessors

    static node_ptr_type next(node_ptr_type n) { return nextlink::load(n); }

/*
    bool is_front(const node_ptr_type node) const
    {
        return node == front_;
    }

    bool is_back(const node_ptr_type node) const
    {
        return nextlink::load(node) == nullptr;
    }

    // identify terminator pointer (it's just a null ptr)

    bool is_end(const node_ptr_type node) const // node points to the element past back
    {
        return (node == nullptr);
    }
*/
};

template<typename NodePtrT, int NEXT_LINK_INDEX>
inline void swap(QwSTailList<NodePtrT, NEXT_LINK_INDEX>& a, QwSTailList<NodePtrT, NEXT_LINK_INDEX>& b)
{
    a.swap(b);
}

#endif /* INCLUDED_QWSTAILLIST_H */

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
