////
//// Created by sancar koyunlu on 5/21/13.
//// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//#ifndef HAZELCAST_QUEUE_BASED_OBJECT_POOL
//#define HAZELCAST_QUEUE_BASED_OBJECT_POOL
//
//#include "ConcurrentQueue.h"
//#include "HazelcastException.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace util {
//            template <typename T, typename Factory>
//            class QueueBasedObjectPool {
//            public:
//                QueueBasedObjectPool(Factory factory)
//                : factory(factory)
//                , active(false){
//
//                };
//
//
//                bool take(T& t) {
//                    if (!active) {
//                        return false;
//                    }
//                    bool b = queue.poll(t);
//                    if (b == false) {
//                        t = factory();
//                    }
//                    return true;
//                }
//
//                void release(T e) {
//                    if (!active || !queue.offer(e)) {
////                        destructor.destroy(e);
//                    }
//                }
//
//                void destroy() {
//                    active = true;
////                    final Collection<E> c = new LinkedList<E>();
////                    queue.drainTo(c);
////                    for (E e : c) {
////                        destructor.destroy(e);
////                    }
//                }
//
//            private:
//                Factory factory;
//                volatile bool active;
//                ConcurrentQueue<T> queue;
//            };
//        }
//    }
//}
//
//#endif //__QueueBasedObjectPool_H_
