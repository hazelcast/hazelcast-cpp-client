/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/protocol/ClientMessage.h"

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
            Future(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &invocationFuture,
                   serialization::pimpl::SerializationService &serializationService,
                   Decoder decoder) : clientInvocationFuture(invocationFuture),
                                      serializationService(serializationService), decoderFunction(decoder) {
            }

            /**
             * This is actually a move constructor
             * Constructs a Future with the shared state of movedFuture using move semantics. After construction,
             * movedFuture.valid() == false.
             */
            Future(const Future &movedFuture) : clientInvocationFuture(movedFuture.clientInvocationFuture),
                                                serializationService(movedFuture.serializationService),
                                                decoderFunction(movedFuture.decoderFunction) {
                const_cast<Future &>(movedFuture).clientInvocationFuture.reset();
            }

            /**
             * Assigns the contents of another future object.
             * 1) Releases any shared state and move-assigns the contents of movedFuture to *this. After the assignment,
             * movedFuture.valid() == false and this->valid() will yield the same value as movedFuture.valid() before
             * the assignment.
             */
            Future &operator=(const Future &movedFuture) {
                this->clientInvocationFuture = movedFuture.clientInvocationFuture;
                const_cast<Future &>(movedFuture).clientInvocationFuture.reset();
                this->decoderFunction = movedFuture.decoderFunction;
                return *this;
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
             * The behaviour is undefined if valid() is false before the call to this function (Our implementation
             * throws FutureUninitialized). Any shared state is
             * released. valid() is false after a call to this method.
             *
             * @return The returned value.
             *
             * @throws one of the hazelcast exceptions, if an exception was stored in the shared state referenced by
             * the future
             */
            std::auto_ptr<V> get() {
                if (!clientInvocationFuture.get()) {
                    throw exception::FutureUninitialized("Future::get", "Future needs to be initialized. "
                            "It may have been moved from.");
                }

                boost::shared_ptr<protocol::ClientMessage> responseMsg = clientInvocationFuture->get();

                assert(responseMsg.get());

                clientInvocationFuture.reset();

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
             * The behaviour is undefined if valid()== false before the call to this function (Our implementation throws
             * FutureUninitialized).
             *
             * @param timeoutInMilliseconds    maximum duration in milliseconds to block for
             */
            future_status wait_for(int64_t timeoutInMilliseconds) const {
                if (!clientInvocationFuture.get()) {
                    throw exception::FutureUninitialized("Future::get", "Future needs to be initialized.");
                }

                try {
                    clientInvocationFuture->get(timeoutInMilliseconds, concurrent::TimeUnit::MILLISECONDS());
                    return future_status::ready;
                } catch (exception::TimeoutException &) {
                    return future_status::timeout;
                }
            }

            /**
             * Blocks until the result becomes available. valid() == true after the call.
             *
             * The behaviour is undefined if valid()== false before the call to this function (Our implementation throws
             * FutureUninitialized).
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
                return clientInvocationFuture.get() != NULL;
            }

        private:
            boost::shared_ptr<spi::impl::ClientInvocationFuture> clientInvocationFuture;
            serialization::pimpl::SerializationService &serializationService;
            Decoder decoderFunction;
        };

        /**
         * This specialization overwrites the get method to return TypedData.
         */
        template<>
        class Future<TypedData> {
        public:
            typedef std::auto_ptr<serialization::pimpl::Data> (*Decoder)(protocol::ClientMessage &response);

            /**
             * This constructor is only for internal use!!!!
             */
            Future(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &invocationFuture,
                   serialization::pimpl::SerializationService &serializationService,
                   Decoder decoder) : clientInvocationFuture(invocationFuture),
                                      serializationService(serializationService), decoderFunction(decoder) {
            }

            /**
             * This is actually a move constructor
             * Constructs a Future with the shared state of movedFuture using move semantics. After construction,
             * movedFuture.valid() == false.
             */
            Future(const Future &movedFuture) : clientInvocationFuture(movedFuture.clientInvocationFuture),
                                                serializationService(movedFuture.serializationService),
                                                decoderFunction(movedFuture.decoderFunction) {
                const_cast<Future &>(movedFuture).clientInvocationFuture.reset();
            }

            /**
             * Assigns the contents of another future object.
             * 1) Releases any shared state and move-assigns the contents of movedFuture to *this. After the assignment,
             * movedFuture.valid() == false and this->valid() will yield the same value as movedFuture.valid() before
             * the assignment.
             */
            Future &operator=(const Future &movedFuture) {
                this->clientInvocationFuture = movedFuture.clientInvocationFuture;
                const_cast<Future &>(movedFuture).clientInvocationFuture.reset();
                this->decoderFunction = movedFuture.decoderFunction;
                return *this;
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
             * The behaviour is undefined if valid() is false before the call to this function (Our implementation
             * throws FutureUninitialized). Any shared state is
             * released. valid() is false after a call to this method.
             *
             * @return The returned value.
             *
             * @throws one of the hazelcast exceptions, if an exception was stored in the shared state referenced by
             * the future
             */
            TypedData get() {
                if (!clientInvocationFuture.get()) {
                    throw exception::FutureUninitialized("Future::get", "Future needs to be initialized. "
                            "It may have been moved from.");
                }

                boost::shared_ptr<protocol::ClientMessage> responseMsg = clientInvocationFuture->get();

                assert(responseMsg.get());

                clientInvocationFuture.reset();

                std::auto_ptr<serialization::pimpl::Data> response = decoderFunction(*responseMsg);

                return TypedData(response, serializationService);
            }

            /**
             * Waits for the result to become available. Blocks until specified timeout_duration has elapsed or the
             * result becomes available, whichever comes first. Returns value identifies the state of the result. A
             * steady clock is used to measure the duration. This function may block for longer than timeout_duration
             * due to scheduling or resource contention delays.
             *
             * The behaviour is undefined if valid()== false before the call to this function (Our implementation throws
             * FutureUninitialized).
             *
             * @param timeoutInMilliseconds    maximum duration in milliseconds to block for
             */
            future_status wait_for(int64_t timeoutInMilliseconds) const {
                if (!clientInvocationFuture.get()) {
                    throw exception::FutureUninitialized("Future::get", "Future needs to be initialized.");
                }

                try {
                    clientInvocationFuture->get(timeoutInMilliseconds, concurrent::TimeUnit::MILLISECONDS());
                    return future_status::ready;
                } catch (exception::TimeoutException &) {
                    return future_status::timeout;
                }
            }

            /**
             * Blocks until the result becomes available. valid() == true after the call.
             *
             * The behaviour is undefined if valid()== false before the call to this function (Our implementation throws
             * FutureUninitialized).
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
                return clientInvocationFuture.get() != NULL;
            }

        private:
            boost::shared_ptr<spi::impl::ClientInvocationFuture> clientInvocationFuture;
            serialization::pimpl::SerializationService &serializationService;
            Decoder decoderFunction;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_FUTURE_H_ */
