//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_SERVER_ERROR
#define HAZELCAST_SERVER_ERROR

#include "hazelcast/client/impl/PortableResponse.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace exception {


            class ServerException : public impl::PortableResponse, public std::exception {

            public:
                ServerException();

                virtual ~ServerException() throw();

                virtual char const *what() const throw();

                std::string message;

                std::string details;

                int type;

                int getClassId() const;

                int getFactoryId() const;

                void readPortable(serialization::PortableReader& reader);
            };
        }
    }
}


#endif //HAZELCAST_SERVER_ERROR
