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

            static std::string INSTANCE_NOT_ACTIVE_STR = "HazelcastInstanceNotActiveException";

            class HAZELCAST_API ServerException : public impl::PortableResponse, public std::exception {

            public:
                ServerException();

                virtual ~ServerException() throw();

                virtual char const *what() const throw();

                int getClassId() const;

                int getFactoryId() const;

                void readPortable(serialization::PortableReader &reader);

                bool isInstanceNotActiveException() const;

            private:
                std::string name;

                std::string details;

                std::string message;

                int type;
            };
        }
    }
}


#endif //HAZELCAST_SERVER_ERROR
