//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_HazelcastIllegalStateException
#define HAZELCAST_HazelcastIllegalStateException

#include "IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            class IllegalStateException : public IException {
            public:
                IllegalStateException(const std::string& source, const std::string& message);

                virtual ~IllegalStateException() throw();

                virtual char const *what() const throw();
            };
        }
    }
}

#endif //HAZELCAST_HazelcastIllegalStateException
