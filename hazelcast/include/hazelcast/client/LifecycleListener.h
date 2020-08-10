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

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/noop.h"
#include "hazelcast/util/type_traits.h"

namespace hazelcast {
    namespace client {
        class LifecycleEvent;
        namespace spi {
            class LifecycleService;
        }

        /**
         * Listener object for listening lifecycle events of hazelcast instance
         *
         * \warning
         * 1 - If listener should do a time consuming operation, off-load the operation to another thread.
         * otherwise it will slow down the system.
         * \warning
         * 2 - Do not make a call to hazelcast. It can cause a deadlock.
         *
         * \see LifecycleEvent
         * \see HazelcastClient::addLifecycleListener
         */
        class HAZELCAST_API LifecycleListener final {
        public:
            /**
             * Set an handler function to be invoked when the state changes
             * \param h a `void` function object that is callable with a single parameter of type `const LifecycleEvent &`
             * \return `*this`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            LifecycleListener &onStateChanged(Handler &&h) & {
                stateChanged = std::forward<Handler>(h);
                return *this;
            }

            /** 
             * \copydoc LifecycleListener::onStateChanged
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            LifecycleListener &&onStateChanged(Handler &&h) && {
                onStateChanged(std::forward<Handler>(h));
                return std::move(*this);
            }
        
        private:
            using HandlerType = std::function<void(const LifecycleEvent &)>;
            HandlerType stateChanged = util::noop<const LifecycleEvent &>;

            friend class spi::LifecycleService;
        };
    }
}




