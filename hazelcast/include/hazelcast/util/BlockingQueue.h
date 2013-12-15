//
// Created by sancar koyunlu on 13/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_BlockingQueue
#define HAZELCAST_BlockingQueue

#include <queue>
#include <thread>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

template <typename T>
class BlockingQueue {
public:

    ~BlockingQueue() {
        boost::unique_lock<boost::mutex> lock(mutex);
        while (!queue.empty()) {
            T *front = queue.front();
            delete front;
            queue.pop();
        }
    };

    template< class Rep, class Period >
    std::auto_ptr<T> pop(T *item, boost::chrono::duration<Rep, Period> duration) {
        boost::unique_lock<boost::mutex> lock(mutex);
        boost::cv_status status;
        if (queue.empty()) {
            status = conditionVariable.wait_for(lock, duration);
            if (status == boost::cv_status::timeout) {
                return std::auto_ptr<T>();
            }
        }
        item = queue.front();
        queue.pop();
        return std::auto_ptr<T>(item);
    }

    std::auto_ptr<T> pop() {
        boost::unique_lock<boost::mutex> lock(mutex);
        while (queue.empty()) {
            conditionVariable.wait(lock);
        }
        T *item = queue.front();
        queue.pop();
        return std::auto_ptr<T>(item);
    }

    void push(T &&item) {
        boost::unique_lock<boost::mutex> lock(mutex);
        queue.push(std::move(item));
        lock.unlock();
        conditionVariable.notify_one();
    }

private:
    std::queue<T *> queue;
    boost::mutex mutex;
    boost::condition_variable conditionVariable;
};

#endif //HAZELCAST_BlockingQueue
