//
// Created by sancar koyunlu on 01/04/14.
//


#ifndef HAZELCAST_Promise
#define HAZELCAST_Promise

#include "hazelcast/util/Future.h"
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace util {
        template <typename T>
        class Promise {
        public:
            Promise()
            :future(new Future<T>()){

            }

            boost::shared_ptr<util::Future<T> > getFuture(){
                return future;
            };

        private:
            boost::shared_ptr<util::Future<T> > future;
        };
    }
}

#endif //HAZELCAST_Promise
