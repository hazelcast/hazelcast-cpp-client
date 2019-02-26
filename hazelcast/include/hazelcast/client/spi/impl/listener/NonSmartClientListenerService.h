/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_NONSMARTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_NONSMARTCLIENTLISTERNERSERVICE_H_

#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    class NonSmartClientListenerService : public AbstractClientListenerService{
                    public:
                        NonSmartClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                      int32_t eventQueueCapacity);

                    protected:
                        virtual bool registersLocalOnly() const;

                    };
                }
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_NONSMARTCLIENTLISTERNERSERVICE_H_
