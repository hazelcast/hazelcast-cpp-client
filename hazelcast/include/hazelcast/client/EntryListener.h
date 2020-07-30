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
#include <utility>

#include "hazelcast/util/empty_function.h"
#include "hazelcast/util/type_traits.h"

namespace hazelcast {
    namespace client {
        class MapEvent;
        class EntryEvent;
        class ReplicatedMap;
        namespace impl {
            template<typename> class EntryEventHandler;
        }

        /**
        * Map entry listener to get notified when a map entry
        * is added, removed, updated, evicted, or expired. 
        * Events will fire as a result
        * of operations carried out via the IMap.
        *
        * \warning
        * 1 - If listener should do a time consuming operation, off-load the operation to another thread.
        * otherwise it will slow down the system.
        * \warning
        * 2 - Do not make a call to hazelcast. It can cause deadlock.
        *
        * \see IMap::addEntryListener
        */
        class EntryListener final {
        public:
            /**
             * Set an handler function to be invoked when an entry is added.
             * \param h a `void` function object that is callable with a single parameter of type `EntryEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_added(Handler &&h) & {
                added = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc EntryListener::on_added
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_added(Handler &&h) && {
                on_added(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when an entry is removed.
             * \param h a `void` function object that is callable with a single parameter of type `EntryEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_removed(Handler &&h) & {
                removed = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc EntryListener::on_removed
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_removed(Handler &&h) && {
                on_removed(std::forward<Handler>(h));
                return std::move(*this);
            }
            
            /**
             * Set an handler function to be invoked when an entry is updated.
             * \param h a `void` function object that is callable with a single parameter of type `EntryEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_updated(Handler &&h) & {
                updated = std::forward<Handler>(h);
                return *this;
            }
            
            /**
             * \copydoc EntryListener::on_updated
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_updated(Handler &&h) && {
                on_updated(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when an entry is evicted.
             * \param h a `void` function object that is callable with a single parameter of type `EntryEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_evicted(Handler &&h) & {
                evicted = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc EntryListener::on_evicted
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_evicted(Handler &&h) && {
                on_evicted(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when an entry is expired.
             * \param h a `void` function object that is callable with a single parameter of type `EntryEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_expired(Handler &&h) & {
                expired = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc EntryListener::on_expired
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_expired(Handler &&h) && {
                on_expired(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked after a WAN replicated entry is merged.
             * \param h a `void` function object that is callable with a single parameter of type `EntryEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_merged(Handler &&h) & {
                merged = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc EntryListener::on_merged
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_merged(Handler &&h) && {
                on_merged(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when all entries are evicted by IMap::evictAll
             * \param h a `void` function object that is callable with a single parameter of type `MapEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_map_evicted(Handler &&h) & {
                map_evicted = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc EntryListener::on_map_evicted
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_map_evicted(Handler &&h) && {
                on_map_evicted(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when all entries are removed by IMap::clear
             * \param h a `void` function object that is callable with a single parameter of type `MapEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &on_map_cleared(Handler &&h) & {
                map_cleared = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc EntryListener::on_map_cleared
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&on_map_cleared(Handler &&h) && {
                on_map_cleared(std::forward<Handler>(h));
                return std::move(*this);
            }

        private: 
            using EntryHandlerType = std::function<void(EntryEvent &&)>;
            static constexpr auto empty_entry_event_handler = util::empty_function<void, EntryEvent &&>;
            EntryHandlerType added = empty_entry_event_handler,
                             removed = empty_entry_event_handler,
                             updated = empty_entry_event_handler,
                             evicted = empty_entry_event_handler, 
                             expired = empty_entry_event_handler,
                             merged = empty_entry_event_handler;

            using MapHandlerType = std::function<void(MapEvent &&)>;
            static constexpr auto empty_map_event_handler = util::empty_function<void, MapEvent &&>;
            MapHandlerType map_evicted = empty_map_event_handler,
                           map_cleared = empty_map_event_handler;

            template<typename>
            friend class impl::EntryEventHandler;
            friend class ReplicatedMap;
        };
    }
}
