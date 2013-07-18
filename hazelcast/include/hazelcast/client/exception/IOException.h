//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_HazelcastIOException
#define HAZELCAST_HazelcastIOException

#include "IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            class IOException : public IException {
            public:
                IOException(const std::string& source, const std::string& message);

                virtual ~IOException() throw();

                virtual char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_HazelcastIOException
