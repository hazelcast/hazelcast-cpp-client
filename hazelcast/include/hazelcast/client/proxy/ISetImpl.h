/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 30/09/14.
//

#ifndef HAZELCAST_ISetImpl
#define HAZELCAST_ISetImpl


#include "hazelcast/client/proxy/ProxyImpl.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API ISetImpl : public ProxyImpl {
            protected:
                ISetImpl(const std::string& instanceName, spi::ClientContext *clientContext);

                std::string addItemListener(impl::BaseEventHandler *handler, bool includeValue);

                bool removeItemListener(const std::string& registrationId);

                int size();

                bool isEmpty();

                bool contains(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data> toArrayData();

                bool add(const serialization::pimpl::Data& element);

                bool remove(const serialization::pimpl::Data& element);

                bool containsAll(const std::vector<serialization::pimpl::Data>& elements);

                bool addAll(const std::vector<serialization::pimpl::Data>& elements);

                bool removeAll(const std::vector<serialization::pimpl::Data>& elements);

                bool retainAll(const std::vector<serialization::pimpl::Data>& elements);

                void clear();

            private:
                int partitionId;
            };
        }
    }
}

#endif //HAZELCAST_ISetImpl
