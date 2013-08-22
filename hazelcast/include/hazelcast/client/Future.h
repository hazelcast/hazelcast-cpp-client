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
                :isValid(false) {

                };

                FutureBase(R *t)
                :isValid(true)
                , result(t) {

                }

                R& get() {
                    wait();
                    return *result;
                };

                bool valid() const {
                    return isValid;
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


            private:
                bool isValid;
                std::auto_ptr< R > result;
                boost::condition_variable condition;
                boost::unique_lock< boost::mutex > lock;
            };

        }
        template<typename R>
        class Future {
            friend class IExecutorService;

        public:
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

            Future(util::AtomicPointer<pImpl::FutureBase<R> > basePtr):
            basePtr(basePtr) {

            };

            void updateBasePtr(util::AtomicPointer<pImpl::FutureBase<R> > basePtr) {
                this->basePtr = basePtr;
            }
        };


    }
}

#endif //HAZELCAST_Future
