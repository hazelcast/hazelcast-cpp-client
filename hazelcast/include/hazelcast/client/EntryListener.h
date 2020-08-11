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

#include "hazelcast/util/noop.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/util/type_traits.h"

namespace hazelcast {
    namespace client {
        class IMap;
        class MapEvent;
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
                add_flag(EntryEvent::type::ADDED);
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
                add_flag(EntryEvent::type::REMOVED);
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
                add_flag(EntryEvent::type::UPDATED);
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
                add_flag(EntryEvent::type::EVICTED);
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
                add_flag(EntryEvent::type::EXPIRED);
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
                add_flag(EntryEvent::type::MERGED);
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
                add_flag(EntryEvent::type::EVICT_ALL);
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
                add_flag(EntryEvent::type::CLEAR_ALL);
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
            static constexpr auto noop_entry_handler = util::noop<EntryEvent &&>;
            EntryHandlerType added = noop_entry_handler,
                             removed = noop_entry_handler,
                             updated = noop_entry_handler,
                             evicted = noop_entry_handler,
                             expired = noop_entry_handler,
                             merged = noop_entry_handler;

            using MapHandlerType = std::function<void(MapEvent &&)>;
            static constexpr auto noop_map_handler = util::noop<MapEvent &&>;
            MapHandlerType map_evicted = noop_map_handler,
                           map_cleared = noop_map_handler;

            int32_t flags = 0;

            void add_flag(EntryEvent::type t) {
                flags |= static_cast<int32_t>(t);
            }

            template<typename>
            friend class impl::EntryEventHandler;
            friend class ReplicatedMap;
            friend class IMap;
        };
    }
}
