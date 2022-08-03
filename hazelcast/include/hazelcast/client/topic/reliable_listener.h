/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <cstdint>

#include "hazelcast/client/exception/iexception.h"
#include "hazelcast/util/noop.h"
#include "hazelcast/util/type_traits.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class reliable_topic;
        namespace topic {
            class message;

            /**
             * Listen to messages from a reliable_topic
             *             
             * <h1>Durable Subscription</h1>
             * reliable_listener allows you to control where you want to start processing a message when the listener is
             * registered. This makes it possible to create a durable subscription by storing the sequence-id of the last message and
             * using this id as the id to start from.
             *
             * <h1>Exception handling</h1>
             * reliable_listener also gives the ability to deal with exceptions via the method reliable_listener::terminate_on_exception.
             *
             * <h1>Global order</h1>
             * The reliable_listener will always get all events in order (global order). It will not get duplicates and
             * there will only be gaps if it is too slow. For more information see {@link #isLossTolerant()}.
             *
             * <h1>Delivery guarantees</h1>
             * Because the reliable_listener controls which item it wants to continue from upon restart, it is very easy to provide
             * an at-least-once or at-most-once delivery guarantee. The function set via reliable_listener::on_store_sequence_id is always 
             * called before a message is processed; so the id can be persisted on some non-volatile storage. When the the stored
             * sequence-id is then passed to reliable_listener::reliable_listener, an at-least-once delivery is implemented since the same 
             * item is now being processed twice. To implement an at-most-once delivery guarantee, add 1 to the stored sequence-id before 
             * passing it to reliable_listener::reliable_listener.
             */
            class HAZELCAST_API reliable_listener final {
                friend class client::reliable_topic;
            public:
                /**
                 * \param loss_tolerant true if this listener is able to deal with message loss. Even though the reliable topic 
                 * promises to be reliable, it can be that the listener is too slow. Eventually the message won't be 
                 * available anymore. If the reliable_listener is not loss tolerant and the topic detects that there are
                 * missing messages, it will terminate the reliable_listener.
                 * \param initial_sequence_id the initial sequence-id from which this listener should start. -1 if there is 
                 * no initial sequence-id and you want to start from the next published message. If you intent to create a 
                 * durable subscriber so you continue from where you stopped the previous time, load the previous 
                 * sequence-id and add 1. If you don't add one, then you will be receiving the same message twice.
                 */
                explicit reliable_listener(bool loss_tolerant, int64_t initial_sequence_id = -1);

                /**
                 * Set an handler function to be invoked when a message is received for the added topic. 
                 * Note that topic guarantees message ordering.
                 * Therefore there is only one thread invoking the function. The user should not keep the thread busy, but preferably
                 * should dispatch it via an Executor. This will increase the performance of the topic.
                 *
                 * \param h a `void` function object that is callable with a single parameter of type `Message &&`
                 * \return `*this`
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                reliable_listener &on_received(Handler &&h) & {
                    received_ = std::move(h);
                    return *this;
                }

                /**
                 * \copydoc reliable_listener::on_received
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                reliable_listener &&on_received(Handler &&h) && {
                    on_received(std::move(h));
                    return std::move(*this);
                }

                /**
                 * Set an handler function to be invoked to informs the listener that it should store the sequence. 
                 * This method is called before the message is processed. Can be used to make a durable subscription.
                 *
                 * \param h a `void` function object that is callable with a single parameter of type `int64_t`
                 * \return `*this`                 
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                reliable_listener &on_store_sequence_id(Handler &&h) & {
                    store_sequence_id_ = std::move(h);
                    return *this;
                }

                /**
                 * \copydoc reliable_listener::on_store_sequence
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                reliable_listener &&on_store_sequence_id(Handler &&h) && {
                    on_store_sequence_id(std::move(h));
                    return std::move(*this);
                }

                /**
                 * Set an handler function that checks if the listener should be terminated based on an exception
                 * thrown while calling the function set by reliable_listener::on_received.
                 *
                 * \param h a `void` function object that is callable with a single parameter of type `const IException &`
                 * \return `*this`
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                reliable_listener &terminate_on_exception(Handler &&h) & {
                    terminal_ = std::move(h);
                    return *this;
                }

                /**
                 * \copydoc reliable_listener::terminate_on_exception
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                reliable_listener &&terminate_on_exception(Handler &&h) && {
                    terminate_on_exception(std::move(h));
                    return std::move(*this);
                }

            private:
                using received_handler_t = std::function<void(message &&)>;
                using store_sequence_id_handler_t = std::function<void(int64_t)>;
                using exception_handler_t = std::function<bool(const exception::iexception &)>;

                bool loss_tolerant_;
                int64_t initial_sequence_id_;

                received_handler_t received_{ util::noop<message &&> };
                store_sequence_id_handler_t store_sequence_id_ { util::noop<int64_t> };
                exception_handler_t terminal_{ 
                    [](const exception::iexception &){
                        return false;
                    }
                };
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



