//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Future
#define HAZELCAST_Future

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/LockGuard.h"
#include <memory>
#include <cassert>

namespace hazelcast {
    namespace util {
        template <typename T>
        class Future {
        public:
            Future()
            : resultReady(false)
            , exceptionReady(false) {

            };

            void set_value(const T &value) {
                LockGuard guard(mutex);
                assert( !(exceptionReady || resultReady) && "Value can not be set twice");
                sharedObject = value;
                resultReady = true;
                conditionVariable.notify_all();
            };

            void set_exception(const client::exception::IException &e) {
                LockGuard guard(mutex);
                assert( !(exceptionReady || resultReady) && "Exception can not be set twice");
                exception = e;
                exceptionReady = true;
                conditionVariable.notify_all();
            };

            T get() {
                LockGuard guard(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    throw exception;
                }
                conditionVariable.wait(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    throw exception;
                }
                assert(false && "InvalidState");
                return sharedObject;
            };

        private:
            bool resultReady;
            bool exceptionReady;
            ConditionVariable conditionVariable;
            Mutex mutex;
            T sharedObject;
            client::exception::IException exception;

            Future(const Future &rhs);

            void operator = (const Future &rhs);
        };
    }
}


#endif //HAZELCAST_Future
