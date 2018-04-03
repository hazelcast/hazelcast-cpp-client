/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/mixedtype/ISet.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string ISet::addItemListener(MixedItemListener &listener, bool includeValue) {
                impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler>(
                                getName(), context->getClientClusterService(),
                                context->getSerializationService(), listener);
                return proxy::ISetImpl::addItemListener(itemEventHandler, includeValue);
            }

            bool ISet::removeItemListener(const std::string &registrationId) {
                return proxy::ISetImpl::removeItemListener(registrationId);
            }

            int ISet::size() {
                return proxy::ISetImpl::size();
            }

            bool ISet::isEmpty() {
                return proxy::ISetImpl::isEmpty();
            }

            std::vector<TypedData> ISet::toArray() {
                return toTypedDataCollection(proxy::ISetImpl::toArrayData());
            }

            void ISet::clear() {
                proxy::ISetImpl::clear();
            }

            ISet::ISet(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ISetImpl(instanceName, context) {
            }
        }
    }
}
