#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\util/ca_auto_ptr.h"
// CA's copy of VC6's auto_ptr (code\util\ca_auto_ptr.h; the original's path, which its __FILE__ gives): it deletes
// with the game's allocator, which needs the including file's THIS_FILE.
#ifndef CA_AUTO_PTR_H
#define CA_AUTO_PTR_H

#include "unsorted.h"

template <class T>
class ca_auto_ptr {
public:
    explicit ca_auto_ptr(T* p = 0) : m_owns(p != 0), m_p(p) {}
    ca_auto_ptr(const ca_auto_ptr<T>& other) : m_owns(other.m_owns), m_p(other.release()) {}
    ca_auto_ptr<T>& operator=(const ca_auto_ptr<T>& other);
    ~ca_auto_ptr();

    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }
    T* get() const { return m_p; }
    T* release() const
    {
        ((ca_auto_ptr<T>*)this)->m_owns = false;
        return m_p;
    }

    bool m_owns;
    T* m_p;
};

template <class T>
ca_auto_ptr<T>& ca_auto_ptr<T>::operator=(const ca_auto_ptr<T>& other)
{
    if (this != &other) {
        if (m_p != other.get()) {
            if (m_owns)
#line 51
                CA_DELETE(m_p);
            m_owns = other.m_owns;
        } else if (other.m_owns) {
            m_owns = true;
        }
        m_p = other.release();
    }
    return *this;
}

template <class T>
ca_auto_ptr<T>::~ca_auto_ptr()
{
    if (m_owns)
        CA_DELETE(m_p);
}

#endif
