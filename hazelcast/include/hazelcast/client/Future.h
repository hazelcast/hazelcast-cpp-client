/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAZELCAST_CLIENT_FUTURE_H_
#define HAZELCAST_CLIENT_FUTURE_H_

#include <stdint.h>
#include <memory>
#include <assert.h>

#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/CallPromise.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class HAZELCAST_API future_status {
        public:
            future_status(int value) : value(value) { }

            enum enum_type {
                ready,
                timeout
            };

            friend bool operator==(future_status lhs, future_status rhs) {
                return enum_type(lhs.value) == enum_type(rhs.value);
            }

            friend bool operator==(future_status lhs, enum_type rhs) { return enum_type(lhs.value) == rhs; }

            friend bool operator==(enum_type lhs, future_status rhs) { return lhs == enum_type(rhs.value); }

            friend bool operator!=(future_status lhs, future_status rhs) {
                return enum_type(lhs.value) != enum_type(rhs.value);
            }

            friend bool operator!=(future_status lhs, enum_type rhs) { return enum_type(lhs.value) != rhs; }

            friend bool operator!=(enum_type lhs, future_status rhs) { return lhs != enum_type(rhs.value); }

            friend bool operator<(future_status lhs, future_status rhs) {
                return enum_type(lhs.value) < enum_type(rhs.value);
            }

            friend bool operator<(future_status lhs, enum_type rhs) { return enum_type(lhs.value) < rhs; }

            friend bool operator<(enum_type lhs, future_status rhs) { return lhs < enum_type(rhs.value); }

            friend bool operator<=(future_status lhs, future_status rhs) {
                return enum_type(lhs.value) <= enum_type(rhs.value);
            }

            friend bool operator<=(future_status lhs, enum_type rhs) { return enum_type(lhs.value) <= rhs; }

            friend bool operator<=(enum_type lhs, future_status rhs) { return lhs <= enum_type(rhs.value); }

            friend bool operator>(future_status lhs, future_status rhs) {
                return enum_type(lhs.value) > enum_type(rhs.value);
            }

            friend bool operator>(future_status lhs, enum_type rhs) { return enum_type(lhs.value) > rhs; }

            friend bool operator>(enum_type lhs, future_status rhs) { return lhs > enum_type(rhs.value); }

            friend bool operator>=(future_status lhs, future_status rhs) {
                return enum_type(lhs.value) >= enum_type(rhs.value);
            }

            friend bool operator>=(future_status lhs, enum_type rhs) { return enum_type(lhs.value) >= rhs; }

            friend bool operator>=(enum_type lhs, future_status rhs) { return lhs >= enum_type(rhs.value); }

        private:
            int value;
        };

        namespace exception {
            class HAZELCAST_API FutureUninitialized : public IException {
            public:
                FutureUninitialized(const std::string &source, const std::string &message) : IException(source,
                                                                                                        message) { }
            };
        }

        /**
         * This is a unique Future. It can not be shared. The copy constructor actually moves and invalidates the moved
         * from Future. It acts similar to std::future (avaialable since C++11)
         *
         * This class in NOT THREAD SAFE. DO NOT use concurrently from multiple threads
         *
         */
        template<typename V>
        class Future {
        public:
            typedef std::auto_ptr<serialization::pimpl::Data> (*Decoder)(protocol::ClientMessage &response);

            /**
             * This constructor is only for internal use!!!!
             */
            Future(connection::CallFuture &callFuture,
                   serialization::pimpl::SerializationService &serializationService,
                   Decoder decoder) : callFuture(new connection::CallFuture(callFuture)),
                                      serializationService(serializationService), decoderFunction(decoder) {
            }

            /**
             * This is actually a move constructor
             *
             */
            Future(const Future &movedFuture) : callFuture(movedFuture.callFuture),
                                                serializationService(movedFuture.serializationService),
                                                decoderFunction(movedFuture.decoderFunction) {
            }

            virtual ~Future() {
            }

            /**
             * The get method waits until the future has a valid result and retrieves it. It effectively calls wait()
             * in order to wait for the result.
             *
             * Important Note: get moves the result at the first call and hence it should not be called more than one
             * time. The second call will result in undefined behaviour.
             *
             * The behavior is undefined if valid() is false before the call to this function. Any shared state is
             * released. valid() is false after a call to this method.
             *
             * @return The returned value.
             *
             * @throws one of the hazelcast exceptions, if an exception was stored in the shared state referenced by
             * the future
             */
            std::auto_ptr<V> get() {
                if (!callFuture.get()) {
                    throw exception::FutureUninitialized("Future::get", "Future needs to be initialized. "
                            "It may have been moved from.");
                }

                std::auto_ptr<protocol::ClientMessage> responseMsg = callFuture->get();

                assert(responseMsg.get());

                callFuture.reset();

                std::auto_ptr<serialization::pimpl::Data> response = decoderFunction(*responseMsg);

                std::auto_ptr<V> result = serializationService.toObject<V>(response.get());

                return result;
            }

            /**
             * Waits for the result to become available. Blocks until specified timeout_duration has elapsed or the
             * result becomes available, whichever comes first. Returns value identifies the state of the result. A
             * steady clock is used to measure the duration. This function may block for longer than timeout_duration
             * due to scheduling or resource contention delays.
             *
             * The behavior is undefined if valid()== false before the call to this function.
             *
             * @param timeoutInMilliseconds    maximum duration in milliseconds to block for
             */
            future_status wait_for(int64_t timeoutInMilliseconds) const {
                if (!callFuture.get()) {
                    throw exception::FutureUninitialized("Future::get", "Future needs to be initialized.");
                }

                return callFuture->waitFor(timeoutInMilliseconds) ? future_status::ready : future_status::timeout;
            }

            /**
             * Blocks until the result becomes available. valid() == true after the call.
             *
             * The behavior is undefined if valid()== false before the call to this function.
             */
            void wait() const {
                wait_for(INT64_MAX);
            }

            /**
             * Checks if the future refers to a shared state.
             *
             * This is the case only for futures that were not moved from until the first time get() is called.
             *
             * @return true if *this refers to a shared state, otherwise false.
             */
            bool valid() const {
                return callFuture.get() != NULL;
            }

        private:
            mutable std::auto_ptr<connection::CallFuture> callFuture;
            serialization::pimpl::SerializationService &serializationService;
            Decoder decoderFunction;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_FUTURE_H_ */
