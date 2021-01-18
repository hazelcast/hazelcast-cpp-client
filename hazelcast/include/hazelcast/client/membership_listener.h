/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <string>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/util/type_traits.h"
#include "hazelcast/util/noop.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        class membership_event;
        class initial_membership_event;

        namespace spi {
            namespace impl {
                class ClientClusterServiceImpl;
            }
        }

        /**
         * Cluster membership listener.
         *
         * The membership_listener will never be called concurrently and all membership_listeners will receive the events
         * in the same order.
         *
         * \warning
         * 1 - If listener should do a time consuming operation, off-load the operation to another thread.
         * otherwise it will slow down the system.
         * \warning
         * 2 - Do not make a call to hazelcast. It can cause deadlock.
         *
         * \see cluster::add_membership_listener
         */
        class HAZELCAST_API membership_listener final {
            friend class cluster;
            friend class spi::impl::ClientClusterServiceImpl;
        public:
            /**
             * Set an handler function to be invoked when a new member joins the cluster.
             * \param h a `void` function object that is callable with a single parameter of type `const MembershipEvent &`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            membership_listener& on_joined(Handler &&h) & {
                joined_ = std::forward<Handler>(h);
                return *this;
            };

            /**
             * \copydoc membership_listener::on_joined
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            membership_listener&& on_joined(Handler &&h) && {
                on_joined(std::forward<Handler>(h));
                return std::move(*this);
            };
            
            /**
             * Set an handler function to be invoked when an existing member leaves the cluster.
             * \param h a `void` function object that is callable with a single parameter of type `const MembershipEvent &`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            membership_listener& on_left(Handler &&h) & {
                left_ = std::forward<Handler>(h);
                return *this;
            };

            /**
             * \copydoc membership_listener::on_left
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            membership_listener&& on_left(Handler &&h) && {
                on_left(std::forward<Handler>(h));
                return std::move(*this);
            };

            /**
             * Set an handler function to be invoked once when this listener is registered.
             * \param h a `void` function object that is callable with a single parameter of type `const InitialMembershipEvent &`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            membership_listener& on_init(Handler &&h) & {
                init_ = std::forward<Handler>(h);
                return *this;
            };

            /**
             * \copydoc membership_listener::on_init
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            membership_listener&& on_init(Handler &&h) && {
                on_init(std::forward<Handler>(h));
                return std::move(*this);
            };

        private:
            using handler_t = std::function<void(const membership_event &)>;
            using init_handler_t = std::function<void(const initial_membership_event &)>;

            handler_t left_{ util::noop<const membership_event &> },
                      joined_{ util::noop<const membership_event &> };
            init_handler_t init_{};
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



