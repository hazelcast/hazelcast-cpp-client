////
//// Created by sancar koyunlu on 6/18/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//#ifndef HAZELCAST_FUTURE
//#define HAZELCAST_FUTURE
//
//#include <memory>
//#include "util/Thread.h"
//#include "IException.h"
//
//namespace hazelcast {
//    namespace client {
//        template<typename R>
//        class Future {
//        public:
//            Future(std::auto_ptr<util::Thread> thread);
//
//            Future(const Future& future);
//
//            std::auto_ptr<R> get() {
//                return std::auto_ptr(thread->join<R>());
//            };
//
//            std::auto_ptr<R> get(useconds_t timeout) {
//                std::auto_ptr<R> result(thread->join<R>());
//                if (result->get() == NULL) {
//                    throw IException("Timeout");
//                } else {
//                    return result;
//                }
//            };
//
//        private:
//            std::auto_ptr<util::Thread> thread;
//        };
//
//    }
//}
//
//
//#endif //HAZELCAST_FUTURE
