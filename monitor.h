//
// All of this code was copied from
// https://stackoverflow.com/questions/12647217/making-a-c-class-a-monitor-in-the-concurrent-sense
// Don't judge me, I already made a lot
//

#ifndef LEAVE_PACKAGE_MONITOR_H
#define LEAVE_PACKAGE_MONITOR_H

#include <iostream>
#include <mutex>

template<class T>
class Monitor
{
public:
    template<typename ...Args>
    explicit Monitor(Args&&... args) : m_cl(std::forward<Args>(args)...){}

    struct Monitor_helper
    {
        explicit Monitor_helper(Monitor* mon) : m_mon(mon), m_ul(mon->m_lock) {}
        T* operator->() { return &m_mon->m_cl;}
        Monitor* m_mon;
        std::unique_lock<std::mutex> m_ul;
    };

    Monitor_helper operator->() { return Monitor_helper(this); }
    Monitor_helper ManuallyLock() { return Monitor_helper(this); }
    T& GetThreadUnsafeAccess() { return m_cl; }

private:
    T           m_cl;
    std::mutex  m_lock;
};

#endif //LEAVE_PACKAGE_MONITOR_H
