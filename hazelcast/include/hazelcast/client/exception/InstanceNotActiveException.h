//
// Created by sancar koyunlu on 27/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_HazelcastInstanceNotActiveException
#define HAZELCAST_HazelcastInstanceNotActiveException

#include "hazelcast/client/exception/IException.h"


namespace hazelcast {
    namespace client {
        namespace exception {
            class HAZELCAST_API InstanceNotActiveException : public IException {
            public:
                InstanceNotActiveException(const std::string &source, const std::string &message);

                virtual ~InstanceNotActiveException() throw();

                virtual char const *what() const throw();
            };

        }
    }
}

#endif //HAZELCAST_HazelcastInstanceNotActiveException
