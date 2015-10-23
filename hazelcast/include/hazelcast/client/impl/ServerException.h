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
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_SERVER_ERROR
#define HAZELCAST_SERVER_ERROR

#include "hazelcast/client/impl/PortableResponse.h"
#include <string>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace impl {
            class ServerException : public impl::PortableResponse {
            public:
                ServerException();

                virtual ~ServerException() throw();

                virtual char const *what() const throw();

                int getClassId() const;

                int getFactoryId() const;

                void readPortable(serialization::PortableReader &reader);

                std::auto_ptr<std::string> name;

                std::auto_ptr<std::string> details;

                std::auto_ptr<std::string> message;
            private:

                int type;
            };
        }
    }
}


#endif //HAZELCAST_SERVER_ERROR

