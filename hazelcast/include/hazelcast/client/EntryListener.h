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

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/util/null_event_handler.h"
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
             * \param h a `void` function object that is callable with a single parameter of type `const EntryEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onEntryAdded(Handler &&h) & {
                entryAdded = std::forward<Handler>(h);
                add_flag(EntryEvent::type::ADDED);
                return *this;
            }

            /**
             * \copydoc EntryListener::onEntryAdded
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onEntryAdded(Handler &&h) && {
                onEntryAdded(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when an entry is removed.
             * \param h a `void` function object that is callable with a single parameter of type `const EntryEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onEntryRemoved(Handler &&h) & {
                entryRemoved = std::forward<Handler>(h);
                add_flag(EntryEvent::type::REMOVED);
                return *this;
            }

            /**
             * \copydoc EntryListener::onEntryRemoved
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onEntryRemoved(Handler &&h) && {
                onEntryRemoved(std::forward<Handler>(h));
                return std::move(*this);
            }
            
            /**
             * Set an handler function to be invoked when an entry is updated.
             * \param h a `void` function object that is callable with a single parameter of type `const EntryEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onEntryUpdated(Handler &&h) & {
                entryUpdated = std::forward<Handler>(h);
                add_flag(EntryEvent::type::UPDATED);
                return *this;
            }
            
            /**
             * \copydoc EntryListener::onEntryUpdated
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onEntryUpdated(Handler &&h) && {
                onEntryUpdated(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when an entry is evicted.
             * \param h a `void` function object that is callable with a single parameter of type `const EntryEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onEntryEvicted(Handler &&h) & {
                entryEvicted = std::forward<Handler>(h);
                add_flag(EntryEvent::type::EVICTED);
                return *this;
            }

            /**
             * \copydoc EntryListener::onEntryEvicted
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onEntryEvicted(Handler &&h) && {
                onEntryEvicted(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when an entry is expired.
             * \param h a `void` function object that is callable with a single parameter of type `const EntryEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onEntryExpired(Handler &&h) & {
                entryExpired = std::forward<Handler>(h);
                add_flag(EntryEvent::type::EXPIRED);
                return *this;
            }

            /**
             * \copydoc EntryListener::onEntryExpired
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onEntryExpired(Handler &&h) && {
                onEntryExpired(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked after a WAN replicated entry is merged.
             * \param h a `void` function object that is callable with a single parameter of type `const EntryEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onEntryMerged(Handler &&h) & {
                entryMerged = std::forward<Handler>(h);
                add_flag(EntryEvent::type::MERGED);
                return *this;
            }

            /**
             * \copydoc EntryListener::onEntryMerged
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onEntryMerged(Handler &&h) && {
                onEntryMerged(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when all entries are evicted by IMap::evictAll
             * \param h a `void` function object that is callable with a single parameter of type `const MapEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onMapEvicted(Handler &&h) & {
                mapEvicted = std::forward<Handler>(h);
                add_flag(EntryEvent::type::EVICT_ALL);
                return *this;
            }

            /**
             * \copydoc EntryListener::onMapEvicted
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onMapEvicted(Handler &&h) && {
                onMapEvicted(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when all entries are removed by IMap::clear
             * \param h a `void` function object that is callable with a single parameter of type `const MapEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &onMapCleared(Handler &&h) & {
                mapCleared = std::forward<Handler>(h);
                add_flag(EntryEvent::type::CLEAR_ALL);
                return *this;
            }

            /**
             * \copydoc EntryListener::onMapCleared
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            EntryListener &&onMapCleared(Handler &&h) && {
                onMapCleared(std::forward<Handler>(h));
                return std::move(*this);
            }

        private: 
            using EntryHandlerType = std::function<void(const EntryEvent &)>;
            static constexpr auto nullEntryEventHandler = util::nullEventHandler<EntryEvent>;
            EntryHandlerType entryAdded = nullEntryEventHandler,
                             entryRemoved = nullEntryEventHandler,
                             entryUpdated = nullEntryEventHandler,
                             entryEvicted = nullEntryEventHandler, 
                             entryExpired = nullEntryEventHandler,
                             entryMerged = nullEntryEventHandler;

            using MapHandlerType = std::function<void(const MapEvent &)>;
            static constexpr auto nullMapEventHandler = util::nullEventHandler<MapEvent>;
            MapHandlerType mapEvicted = nullMapEventHandler,
                           mapCleared = nullMapEventHandler;

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
