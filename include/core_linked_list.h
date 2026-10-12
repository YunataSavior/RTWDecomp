#line 2 "ca_libs\\ca_include\\core_linked_list.h"
// CA's doubly linked list (ca_libs\ca_include\core_linked_list.h; the original's path, which its __FILE__s give).
// Its functions are compiled in each file that uses them, and allocate with that file's THIS_FILE as their owner.
#ifndef CORE_LINKED_LIST_H
#define CORE_LINKED_LIST_H

#include "unsorted.h"

template <class T>
struct ListNode {
    ListNode(const T& data, ListNode* prev = 0) : m_data(data), m_prev(prev), m_next(0) {}

    T m_data;
    ListNode* m_prev;
    ListNode* m_next;
};

template <class T>
class LinkedList {
public:
    typedef ListNode<T> Node;

    class iterator {
    public:
        iterator() : m_node(0) {}
        iterator(Node* node) : m_node(node) {}
        T& operator*() const { return m_node->m_data; }
        iterator& operator++() { m_node = m_node->m_next; return *this; }
        bool operator==(const iterator& other) const { return m_node == other.m_node; }
        bool operator!=(const iterator& other) const { return !(*this == other); }

        Node* m_node;
    };

    LinkedList() { m_head = 0; m_tail = 0; }
    ~LinkedList();

    int size()
    {
        int count = 0;
        for (iterator it = head(); it != iterator(); ++it)
            count++;
        return count;
    }
    T& operator[](int index)
    {
        iterator it = head();
        while (index--)
            ++it;
        return *it;
    }
    iterator erase(iterator& it);
    iterator remove(iterator& it);

    Node* head() { return m_head; }
    iterator begin() { return head(); }
    Node* tail() { return m_tail; }
    iterator last() { return m_tail; }
    Node* push_back(const T& item) { return append(item).tail(); }
    LinkedList& append(const T& item);
    LinkedList& pop_back();
    void remove_last() { pop_back(); }

    Node* m_head;
    Node* m_tail;
};

template <class T>
LinkedList<T>::~LinkedList()
{
    while (m_head) {
        m_tail = m_head->m_next;
#line 144
        CA_DELETE(m_head);
        m_head = m_tail;
    }
    m_head = 0;
    m_tail = 0;
}

template <class T>
LinkedList<T>& LinkedList<T>::append(const T& item)
{
    if (m_tail) {
#line 226
        Node* node = new (THIS_FILE, __FILE__, __LINE__, true) Node(item, m_tail);
        m_tail->m_next = node;
        m_tail = node;
    } else {
#line 233
        Node* first = new (THIS_FILE, __FILE__, __LINE__, true) Node(item);
        m_head = first;
        m_tail = first;
    }
    return *this;
}

template <class T>
LinkedList<T>& LinkedList<T>::pop_back()
{
    Node* node;
    if (m_tail->m_prev) {
        node = m_tail;
        m_tail->m_prev->m_next = 0;
        m_tail = node->m_prev;
#line 330
        CA_DELETE(node);
    } else {
#line 335
        CA_DELETE(m_tail);
        m_tail = 0;
        m_head = 0;
    }
    return *this;
}

template <class T>
LinkedList<T>::iterator LinkedList<T>::erase(iterator& it)
{
    iterator next = it.m_node->m_next;
    if (m_head == it.m_node) {
        if (m_tail == it.m_node) {
            m_tail = 0;
            m_head = 0;
        } else {
            m_head = it.m_node->m_next;
            it.m_node->m_next->m_prev = 0;
        }
    } else if (m_tail == it.m_node) {
        m_tail = it.m_node->m_prev;
        it.m_node->m_prev->m_next = 0;
    } else {
        it.m_node->m_prev->m_next = it.m_node->m_next;
        it.m_node->m_next->m_prev = it.m_node->m_prev;
    }
#line 529
    CA_DELETE(it.m_node);
    return next;
}

template <class T>
LinkedList<T>::iterator LinkedList<T>::remove(iterator& it)
{
    iterator next = it.m_node->m_next;
    if (m_head == it.m_node) {
        if (m_tail == it.m_node) {
            m_tail = 0;
            m_head = 0;
        } else {
            m_head = it.m_node->m_next;
            it.m_node->m_next->m_prev = 0;
        }
    } else if (m_tail == it.m_node) {
        m_tail = it.m_node->m_prev;
        it.m_node->m_prev->m_next = 0;
    } else {
        it.m_node->m_prev->m_next = it.m_node->m_next;
        it.m_node->m_next->m_prev = it.m_node->m_prev;
    }
#line 564
    CA_DELETE(it.m_node);
    return next;
}

// Something told of an event (the first virtual function of its class).
template <class T>
class Observer {
public:
    virtual void notify(const T& event) = 0;
};

// The observers of an object's events. The observers being told are kept on a stack, so that an observer can remove
// itself (or another) while it is told.
template <class T>
class ObserverList {
public:
    typedef LinkedList<Observer<T>*> List;

    void notify(const T& event);

    List m_observers;
    LinkedList<List::iterator> m_telling;
};

template <class T>
void ObserverList<T>::notify(const T& event)
{
    List::iterator it(m_observers.head());
    while (it != List::iterator()) {
        m_telling.push_back(it);
        (*it)->notify(event);
        it = *m_telling.last();
        m_telling.remove_last();
        if (it != List::iterator())
            ++it;
        else
            break;
    }
}

#endif
