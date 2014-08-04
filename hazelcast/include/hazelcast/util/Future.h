//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Future
#define HAZELCAST_Future

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/client/exception/pimpl/ExceptionHandler.h"
#include "hazelcast/util/ILogger.h"
#include <memory>
#include <cassert>

namespace hazelcast {
    namespace util {
        template <typename T>
        class HAZELCAST_API Future {
        public:
            Future()
            : resultReady(false)
            , exceptionReady(false) {

            };

            void set_value(const T &value) {
                LockGuard guard(mutex);
                if( exceptionReady || resultReady ) {
                    util::ILogger::getLogger().warning(std::string("Future.set_value should not be called twice" ) );
                    return;
                }
                sharedObject = value;
                resultReady = true;
                conditionVariable.notify_all();
            };

            void set_exception(const std::string &exceptionName, const std::string &exceptionDetails) {
                LockGuard guard(mutex);
                if( exceptionReady || resultReady ) {
                    util::ILogger::getLogger().warning(std::string("Future.set_exception should not be called twice : details" ) + exceptionDetails);
                    return;
                }
                this->exceptionName = exceptionName;
                this->exceptionDetails = exceptionDetails;
                exceptionReady = true;
                conditionVariable.notify_all();
            };

            T get() {
                LockGuard guard(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    client::exception::pimpl::ExceptionHandler::rethrow(exceptionName, exceptionDetails);
                }
                conditionVariable.wait(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    client::exception::pimpl::ExceptionHandler::rethrow(exceptionName, exceptionDetails);
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
            std::string exceptionName;
            std::string exceptionDetails;

            Future(const Future &rhs);

            void operator = (const Future &rhs);
        };
    }
}


#endif //HAZELCAST_Future

