//
// Created by sancar koyunlu on 7/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "deneme.h"
#include "AtomicPointer.h"
#include "Member.h"
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <fstream>

namespace ttt {

    template <typename T>
    class AtomicPointer {
    public:
        AtomicPointer()
        :pointer(NULL), accessLock(new boost::mutex), localLock(new boost::mutex) {
        };

        AtomicPointer(T *const p)
        :pointer(p), accessLock(new boost::mutex), localLock(new boost::mutex) {

        };

        AtomicPointer(const AtomicPointer &rhs) {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*(rhs.accessLock));
            pointer = rhs.pointer;
            accessLock = rhs.accessLock;
        };

        ~AtomicPointer() {

        };

        void operator = (const AtomicPointer &rhs) {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*(rhs.accessLock));
            pointer = rhs.pointer;
            accessLock = rhs.accessLock;
        };


        bool operator ==(const AtomicPointer &rhs) {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*(accessLock));
            return pointer == rhs.pointer;
        };


        T operator *() {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return *pointer;
        };

        T *operator ->() {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return pointer.get();
        };

        T *get() {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return pointer.get();
        };

        void reset(T *p) {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*accessLock);
            pointer.reset(p);
        };

        int use_count() {
            boost::lock_guard<boost::mutex> lg2(*localLock);
            boost::lock_guard<boost::mutex> lg(*accessLock);
            return pointer.use_count();
        }
    private:
        boost::shared_ptr<T> pointer;
        boost::shared_ptr<boost::mutex> accessLock;
        std::auto_ptr<boost::mutex> localLock;
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


namespace atomicSafety {
    ttt::AtomicPointer<int> testptr;

    void update(ttt::AtomicPointer<int>& xx) {
        while (true) {
            testptr = xx;
        }
    };

    void observer(std::vector< ttt::AtomicPointer<int> * >& s) {
        unsigned long size = s.size();
        std::vector<int *> ptrs(size);
        for (int i = 0; i < size; i++) {
            ptrs[i] = s[i]->get();
        }

        while (true) {
            int *x = testptr.get();
            bool equalToOne = false;
            for (int i = 0; i < size; i++) {
                if (ptrs[i] == x) {
                    equalToOne = true;
                }
            }
            if (!equalToOne) {
                std::cout << x << std::endl;
                std::cout.flush();
            }
        };
    }


    void test() {
        std::vector< ttt::AtomicPointer<int> * > atomicPtrs;

        for (int i = 0; i < 20; i++) {
            ttt::AtomicPointer<int> *atomicPointer = new ttt::AtomicPointer<int>(new int);
            atomicPtrs.push_back(atomicPointer);
            boost::thread updateThread(update, boost::ref(*atomicPointer));
        }

        testptr = *(atomicPtrs[0]);
        boost::thread t(observer, boost::ref(atomicPtrs));
        t.join();
    }
}

namespace  atomicVisibility {
    ttt::AtomicPointer<int> testptr;

    void update() {
        int i = 20000;
        int last;
        while (i--) {
            int *pInt = new int;

            *pInt = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
            ttt::AtomicPointer<int> *newValue = new ttt::AtomicPointer<int>(pInt);
            testptr = *newValue;
            last = *pInt;
        }
        std::cout << "update finished " << last << std::endl;
    };

    void observer() {
        try{
            std::ofstream file;
            boost::thread::id s = boost::this_thread::get_id();
            std::stringstream id;
            id << "t" << s << ".txt";
            file.open(id.str().c_str(), std::ofstream::out);
            int i = 10000;
            while (i--) {
                int *pInt = testptr.get();
                file << *pInt << "\t:\t" << pInt << std::endl;
            }
            std::cout.flush();
            file.flush();
            file.close();
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        } catch(...){
            std::cout << "unknown exception" << std::endl;
        }
        std::cout << "finished" << std::endl;
    }


    void test() {
        int *p = new int;
        *p = 1;
        testptr.reset(p);

        boost::thread o(observer);
        boost::thread o1(observer);
        boost::thread o2(observer);
        boost::thread o3(observer);
        boost::thread o4(observer);
        boost::thread u(update);
        std::cout << "Joining" << std::endl;
        o.join();
        o1.join();
        o2.join();
        o3.join();
        o4.join();
        u.join();

    }
}

int deneme::init() {
//    atomicSafety::test();
//    atomicVisibility::test();
    return 0;
}
