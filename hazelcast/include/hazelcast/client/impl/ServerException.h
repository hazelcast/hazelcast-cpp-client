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

