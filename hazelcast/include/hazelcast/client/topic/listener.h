/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <functional>

#include "hazelcast/util/export.h"
#include "hazelcast/util/type_traits.h"
#include "hazelcast/util/noop.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            class message;
            namespace impl {
                class TopicEventHandlerImpl;
            }

            /**
             * Listen to messages from an ITopic
             * \see ITopic::add_message_listener
             */
            class HAZELCAST_API listener final {
            public:
                /**
                 * Set an handler function to be invoked when a message is received for the subscribed topic.
                 *
                 * \warning 
                 * In order to guarantee message ordering, there is only one thread that invokes the given function.
                 * The user should off-load any time consuming operation to another thread.
                 *
                 * \param h a `void` function object that is callable with a single parameter of type `Message &&`
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                listener &on_received(Handler &&h) & {
                    received_ = std::move(h);
                    return *this;
                }

                /**
                 * \copydoc Listener::on_received
                 */
                template<typename Handler,
                         typename = util::enable_if_rvalue_ref_t<Handler &&>>
                listener &&on_received(Handler &&h) && {
                    on_received(std::move(h));
                    return std::move(*this);
                }

            private:
                using HandlerType = std::function<void(message &&)>;

                HandlerType received_ = util::noop<message &&>;

                friend class impl::TopicEventHandlerImpl;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



