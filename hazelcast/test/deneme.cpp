//
// Created by sancar koyunlu on 7/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include <ostream>
#include <iostream>
#include "deneme.h"
#include "AtomicPointer.h"
#include "Member.h"
#include <boost/thread.hpp>
#include <boost/atomic.hpp>


//const int threadCount = 100;
//
//std::vector<boost::shared_ptr<int> > hold[threadCount];
//
//void copySharedPtr(boost::shared_ptr<int> *ptr, int id) {
//    hold[id].resize(1000);
//
//    for (int i = 0; i < 1000; i++) {
//        hold[id][i] = *ptr;
//    }
//    boost::this_thread::sleep(boost::posix_time::seconds(3));
//
//    for (int j = 0; j < 1000; j++) {
//        assert(1000 * threadCount + 1 == hold[id][j].use_count() );
//    }
//}

//void atomicPointerTest() {
//    boost::shared_ptr<int> ptr(new int);
//    for (int i = 0; i < threadCount; i++) {
//        boost::thread s3(copySharedPtr, &ptr, i);
//
//    }
//    boost::this_thread::sleep(boost::posix_time::seconds(3));
//    long count = ptr.use_count();
//    std::cout << count << std::endl;
//    for (int i = 0; i < threadCount; i++) {
//        for (int j = 0; j < 1000; j++) {
//            assert(count == hold[i][j].use_count() );
//        }
//    }
//
//}

namespace ttt {

    template <typename T>
    class AtomicPointer {
    public:
        AtomicPointer()
        :pointer(NULL), accessLock(new boost::mutex) {
        };

        AtomicPointer(T *const p)
        :pointer(p), accessLock(new boost::mutex) {

        };

        AtomicPointer(AtomicPointer &rhs) {
            boost::lock_guard<boost::mutex> lg(*(rhs.accessLock));
            pointer = rhs.pointer;
            accessLock = rhs.accessLock;
        };

        bool operator ==(const AtomicPointer &rhs) {
            boost::lock_guard<boost::mutex> lg(*(accessLock));
            return pointer == rhs.pointer;
        };

        void operator = (const AtomicPointer &rhs) {
            boost::lock_guard<boost::mutex> lg(*(rhs.accessLock));
            pointer = rhs.pointer;
            accessLock = rhs.accessLock;
        }

        T operator *() {
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return *pointer;
        };

        T *operator ->() {
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return pointer.get();
        };

        T *get() {
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return pointer.get();
        };

        void reset(T *p) {
            boost::lock_guard<boost::mutex> lg(*accessLock);
            pointer.reset(p);
        };

        int use_count() {
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return pointer.use_count();
        }

    private:
        boost::shared_ptr<T> pointer;
        boost::shared_ptr<boost::mutex> accessLock;
    };

    template<typename T>
    inline std::ostream& operator <<(std::ostream &strm, ttt::AtomicPointer<T> &a) {
        return strm << std::string("AtomicPointer[") << a.get() << "]:use_count[" << hazelcast::util::to_string(a.use_count()) << "]";
    };
//
//    template <typename K, typename V, typename Comparator  = std::less<K> >
//    class ConcurrentSmartMap {
//    public:
//        ConcurrentSmartMap() {
//
//        };
//
//        ~ConcurrentSmartMap() {
//        };
//
//        bool containsKey(const K& key) const {
//            boost::lock_guard<boost::mutex> guard (mapLock);
//            return internalMap.count(key) > 0;
//        };
//
//        /**
//         *
//         * @return the previous value associated with the specified key,
//         *         or <tt>null</tt> if there was no mapping for the key
//         */
//        AtomicPointer<V> putIfAbsent(const K& key, AtomicPointer<V> value) {
//            boost::lock_guard<boost::mutex> lg(mapLock);
//            if (internalMap.count(key) > 0) {
//                return internalMap[key];
//            } else {
//                internalMap[key] = value;
//                return NULL;
//            }
//        };
//
//        AtomicPointer<V> put(const K& key, V *value) {
//            boost::lock_guard<boost::mutex> lg(mapLock);
//            if (internalMap.count(key) > 0) {
//                AtomicPointer<V> tempValue = internalMap[key];
//                internalMap[key] = AtomicPointer<V>(value);
//                return tempValue;
//            } else {
//                internalMap[key] = AtomicPointer<V>(value);
//                return NULL;
//            }
//        };
//
//        /**
//         * Returns the value to which the specified key is mapped,
//         * or {@code null} if this map contains no mapping for the key.
//         *
//         */
//        AtomicPointer<V> get(const K& key) {
//            boost::lock_guard<boost::mutex> lg(mapLock);
//            if (internalMap.count(key) > 0)
//                return internalMap[key];
//            else
//                return NULL;
//        };
//
//        AtomicPointer<V> remove(const K& key) {
//            boost::lock_guard<boost::mutex> lg(mapLock);
//            if (internalMap.count(key) > 0) {
//                AtomicPointer<V> value = internalMap[key];
//                internalMap.erase(internalMap.find(key));
//                return value;
//            }
//            else
//                return NULL;
//        };
//
//        std::vector<AtomicPointer<V> > values() {
//            boost::lock_guard<boost::mutex> lg(mapLock);
//            std::vector<AtomicPointer<V>> val(internalMap.size());
//            int i = 0;
//            for (typename std::map<K, AtomicPointer<V>>::iterator it = internalMap.begin(); it != internalMap.end(); ++it) {
//                val[i++] = it->second;
//            }
//            return val;
//        };
//
//        std::vector<K> keys() {
//            boost::lock_guard<boost::mutex> lg(mapLock);
//            std::vector<K> k(internalMap.size());
//            int i = 0;
//            for (typename std::map<K, V *>::iterator it = internalMap.begin(); it != internalMap.end(); ++it) {
//                k[i++] = it->first;
//            }
//            return k;
//        };
//
//    private:
//        std::map<K, AtomicPointer<V>, Comparator> internalMap;
//        mutable boost::mutex mapLock;
//
//    };


}

ttt::AtomicPointer<int> testptr;
ttt::AtomicPointer<int> a(new int);
ttt::AtomicPointer<int> b(new int);
boost::mutex printLock;

void printTestPtrs(boost::thread::id i) {
//    printLock.lock();
    std::cout << i << std::endl;
    std::cout << testptr << std::endl;
    std::cout << a << std::endl;
    std::cout << b << std::endl;
    std::cout.flush();
//    printLock.unlock();
};

void update(ttt::AtomicPointer<int>& xx) {
    while (true) {
//        boost::this_thread::sleep(boost::posix_time::seconds(1));
        testptr = xx;
//        printTestPtrs(boost::this_thread::get_id());

    }
};

void observer() {
    int *a_ptr = a.get();
    int *b_ptr = b.get();

    boost::this_thread::sleep(boost::posix_time::seconds(20));
    while (true) {
        int *x = testptr.get();
        if (!(a_ptr == x || b_ptr == x || x == NULL)) {
            std::cout << x << std::endl;
            std::cout.flush();
        }
//        printTestPtrs(boost::this_thread::get_id());
    };
}


int deneme::init() {
    boost::thread t(observer);
    boost::thread x(update, boost::ref(a));
    boost::thread y(update, boost::ref(b));
    t.join();
    return 0;
}
