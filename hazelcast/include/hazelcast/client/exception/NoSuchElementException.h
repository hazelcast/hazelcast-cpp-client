//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_NoSuchElementException
#define HAZELCAST_NoSuchElementException

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            class HAZELCAST_API NoSuchElementException : public IException {
            public:
                NoSuchElementException(const std::string& source, const std::string& message);

                virtual ~NoSuchElementException() throw();

                virtual char const *what() const throw();
            };
        }
    }
}

#endif //HAZELCAST_NoSuchElementException
