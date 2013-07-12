//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_InterruptedException
#define HAZELCAST_InterruptedException

#include "IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            class InterruptedException : public IException {
            public:
                InterruptedException(const std::string& source, const std::string& message);

                virtual ~InterruptedException() throw();

                virtual char const *what() const throw();
            };
        }
    }
}



#endif //HAZELCAST_InterruptedException
