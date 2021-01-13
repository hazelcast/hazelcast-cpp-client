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
#include "hazelcast/client/entry_event.h"
#include "hazelcast/util/type_traits.h"

namespace hazelcast {
    namespace client {
        class imap;
        class map_event;
        class replicated_map;
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
        * \see imap::add_entry_listener
        */
        class entry_listener final {
        public:
            /**
             * Set an handler function to be invoked when an entry is added.
             * \param h a `void` function object that is callable with a single parameter of type `EntryEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &on_added(Handler &&h) & {
                added_ = std::forward<Handler>(h);
                add_flag(entry_event::type::ADDED);
                return *this;
            }

            /**
             * \copydoc entry_listener::on_added
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_added(Handler &&h) && {
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
            entry_listener &on_removed(Handler &&h) & {
                removed_ = std::forward<Handler>(h);
                add_flag(entry_event::type::REMOVED);
                return *this;
            }

            /**
             * \copydoc entry_listener::on_removed
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_removed(Handler &&h) && {
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
            entry_listener &on_updated(Handler &&h) & {
                updated_ = std::forward<Handler>(h);
                add_flag(entry_event::type::UPDATED);
                return *this;
            }
            
            /**
             * \copydoc entry_listener::on_updated
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_updated(Handler &&h) && {
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
            entry_listener &on_evicted(Handler &&h) & {
                evicted_ = std::forward<Handler>(h);
                add_flag(entry_event::type::EVICTED);
                return *this;
            }

            /**
             * \copydoc entry_listener::on_evicted
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_evicted(Handler &&h) && {
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
            entry_listener &on_expired(Handler &&h) & {
                expired_ = std::forward<Handler>(h);
                add_flag(entry_event::type::EXPIRED);
                return *this;
            }

            /**
             * \copydoc entry_listener::on_expired
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_expired(Handler &&h) && {
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
            entry_listener &on_merged(Handler &&h) & {
                merged_ = std::forward<Handler>(h);
                add_flag(entry_event::type::MERGED);
                return *this;
            }

            /**
             * \copydoc entry_listener::on_merged
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_merged(Handler &&h) && {
                on_merged(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when all entries are evicted by imap::evict_all
             * \param h a `void` function object that is callable with a single parameter of type `MapEvent &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &on_map_evicted(Handler &&h) & {
                map_evicted_ = std::forward<Handler>(h);
                add_flag(entry_event::type::EVICT_ALL);
                return *this;
            }

            /**
             * \copydoc entry_listener::on_map_evicted
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_map_evicted(Handler &&h) && {
                on_map_evicted(std::forward<Handler>(h));
                return std::move(*this);
            }

            /**
             * Set an handler function to be invoked when all entries are removed by imap::clear
             * \param h a `void` function object that is callable with a single parameter of type `map_event &&`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &on_map_cleared(Handler &&h) & {
                map_cleared_ = std::forward<Handler>(h);
                add_flag(entry_event::type::CLEAR_ALL);
                return *this;
            }

            /**
             * \copydoc entry_listener::on_map_cleared
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            entry_listener &&on_map_cleared(Handler &&h) && {
                on_map_cleared(std::forward<Handler>(h));
                return std::move(*this);
            }

        private: 
            using EntryHandlerType = std::function<void(entry_event &&)>;
            EntryHandlerType added_ = util::noop<entry_event &&>,
                             removed_ = util::noop<entry_event &&>,
                             updated_ = util::noop<entry_event &&>,
                             evicted_ = util::noop<entry_event &&>,
                             expired_ = util::noop<entry_event &&>,
                             merged_ = util::noop<entry_event &&>;

            using MapHandlerType = std::function<void(map_event &&)>;
            MapHandlerType map_evicted_ = util::noop<map_event &&>,
                           map_cleared_ = util::noop<map_event &&>;

            int32_t flags_ = 0;

            void add_flag(entry_event::type t) {
                flags_ |= static_cast<int32_t>(t);
            }

            template<typename>
            friend class impl::EntryEventHandler;
            friend class replicated_map;
            friend class imap;
        };
    }
}
