/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 11/02/14.
//


#ifndef HAZELCAST_BaseRemoveListenerRequest
#define HAZELCAST_BaseRemoveListenerRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }
        namespace impl {
            class BaseRemoveListenerRequest : public impl::ClientRequest {
            public:
                BaseRemoveListenerRequest(const std::string &name, const std::string registrationId);

                virtual ~BaseRemoveListenerRequest();

                void setRegistrationId(const std::string &registrationId);

                virtual void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                std::string registrationId;
            };

        }
    }
}

#endif //HAZELCAST_BaseRemoveListenerRequest

