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

#include <utility>
#include <functional>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/noop.h"
#include "hazelcast/util/type_traits.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class ItemEvent;

        namespace impl {
            template<typename> class ItemEventHandler;
        }

        /**
        * Item listener for IQueue, ISet and IList
        *
        * \warning 
        * 1 - If listener should do a time consuming operation, off-load the operation to another thread.
        * Otherwise it will slow down the system.
        * \warning
        * 2 - Do not make a call to hazelcast. It can cause a deadlock.
        *
        * \see IList::addItemListener
        * \see IQueue::addItemListener
        * \see ISet::addItemListener
        */
        class HAZELCAST_API ItemListener final {
        public:
            /**
             * Set an handler function to be invoked when an item is added
             * \param h a `void` function object that is callable with a single parameter of type `ItemEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            ItemListener &on_added(Handler &&h) & {
                added = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc ItemListener::on_added
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            ItemListener &&on_added(Handler &&h) && {
                on_added(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when an item is removed
             * \param h a `void` function object that is callable with a single parameter of type `ItemEvent &&`
             */
            template<typename Handler>
            ItemListener &on_removed(Handler &&h) & {
                removed = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc ItemListener::on_removed
             */
            template<typename Handler>
            ItemListener &&on_removed(Handler &&h) && {
                on_removed(std::forward<Handler>(h));
                return std::move(*this);
            }

        private:
            using HandlerType = std::function<void(ItemEvent &&)>;
            static constexpr auto noop_handler = util::noop<ItemEvent &&>;
            HandlerType added = noop_handler,
                        removed = noop_handler;

            template<typename>
            friend class impl::ItemEventHandler;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
