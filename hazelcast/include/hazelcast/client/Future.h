//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#include "boost/thread.hpp"
#include "AtomicPointer.h"
#include <memory>

#ifndef HAZELCAST_Future
#define HAZELCAST_Future

namespace hazelcast {
    namespace client {

        class FutureStatus {
        public:
            enum Status {
                READY,
                TIMEOUT
            } value;

            FutureStatus(Status status)
            :value(status) {

            };

            bool operator ==(Status status) {
                return this->value == status;
            };

            bool operator !=(Status status) {
                return !(*this == status);
            };
        };

        namespace pImpl {
            template<typename R>
            class FutureBase {
            public:
                FutureBase()
                :result(NULL)
                , exception(NULL) {

                };

                R& get() {
                    wait();
                    if (exception != NULL)
                        throw exception;
                    return *result;
                };

                bool valid() const {
                    return result != NULL;
                };

                void wait() {
                    condition.wait(lock);
                };

                FutureStatus wait_for(long timeInMillis) {
                    boost::cv_status status = condition.wait_for(lock, boost::chrono::milliseconds(timeInMillis));
                    if (status == boost::cv_status::timeout)
                        return FutureStatus::TIMEOUT;
                    else
                        return FutureStatus::READY;
                };


                void setValue(R *value) {
                    result.reset(value);
                    condition.notify_all();
                };

                void setException(std::exception *exception) {
                    this->exception.reset(exception);
                    condition.notify_all();
                }

            private:
                bool isValid;
                std::auto_ptr< R > result;
                std::auto_ptr< std::exception> exception;
                boost::condition_variable condition;
                boost::unique_lock< boost::mutex > lock;

            };

        }
        template<typename R>
        class Future {
        public:
            Future():
            basePtr(new pImpl::FutureBase<R>) {

            };

            pImpl::FutureBase<R>& operator ->() {
                return *(basePtr.get());
            }

            R& get() {
                return basePtr->get();
            };

            bool valid() const {
                return basePtr->valid();
            };

            void wait() {
                basePtr->wait();
            };

            FutureStatus wait_for(long timeInMillis) {
                return basePtr->wait_for(timeInMillis);
            };

        private:
            util::AtomicPointer<pImpl::FutureBase<R> > basePtr;


        };


    }
}

#endif //HAZELCAST_Future
