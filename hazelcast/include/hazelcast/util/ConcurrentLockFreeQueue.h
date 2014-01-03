////
//// Created by sancar koyunlu on 19/12/13.
////
//
//
//#ifndef HAZELCAST_ConcurrentLockFreeQueue
//#define HAZELCAST_ConcurrentLockFreeQueue
//
//#include <boost/atomic.hpp>
//
//
//template <typename T>
//class ConcurrentLockFreeQueue {
//    struct Node;
//
//    class Pointer {
//    public:
//        Pointer()
//        :ptr(NULL)
//        , count(0) {
//
//        };
//
//        Pointer(Node *ptr, int count)
//        :ptr(ptr)
//        , count(count) {
//
//        }
//
//        Node *ptr;
//        int count;
//    };
//
//    bool isEqual(const Pointer a, const Pointer b) {
//        return a.ptr == b.ptr && a.count == b.count;
//    };
//    struct Node {
//        T *value;
//        boost::atomic<Pointer> next;
//    };
//
//    template<typename P>
//    static bool CAS(boost::atomic<P> &l, P &r, Pointer p) {
//        return l.compare_exchange_strong(r, p);
//    }
//
//
//public:
//    ConcurrentLockFreeQueue() {
//        Node *dummyNode = new Node();
//        Pointer dummyPointer(dummyNode, 0);
//        _head.store(dummyPointer);
//        _tail.store(dummyPointer);
//    }
//
//    void push(T *const value) {
//        Node *node = new Node();
//        node->value = value;
//        for (; ;) {
//            Pointer tail = _tail.load();
//            Pointer next = tail.ptr->next.load();
//            if (isEqual(tail, _tail.load())) {//?
//                if (next.ptr == NULL) {
//                    if (CAS(tail.ptr->next, next, Pointer(node, next.count + 1))) {
//                        break;
//                    }
//                } else {
//                    CAS(_tail, tail, Pointer(next.ptr, _tail.load().count + 1));
//                }
//            }
//            CAS(_tail, tail, Pointer(node, tail.count + 1));
//        }
//    }
//
//    bool pop(T *& pValue) {
//        for (; ;) {
//            Pointer head = _head;
//            Pointer tail = _tail;
//            Pointer next = _head.load().ptr->next;
//            if (isEqual(head, _head.load())) {
//                if (_head.load().ptr == _tail.load().ptr) {
//                    if (next.ptr == NULL) {
//                        return false;
//                    }
//                    CAS(_tail, tail, Pointer(next.ptr, _tail.load().count + 1));
//                } else {
//                    pValue = next.ptr->value;
//                    if (CAS(_head, tail, Pointer(next.ptr, _head.load().count + 1)))
//                        break;
//                }
//            }
//        }
//        delete _head.load().ptr;
//        return true;
//    }
//
//
//private:
//    boost::atomic<Pointer> _head;
//    boost::atomic<Pointer> _tail;
//};
//
//
//#endif //HAZELCAST_ConcurrentLockFreeQueue
