//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_HazelcastIOException
#define HAZELCAST_HazelcastIOException

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Raised when an Input Output error is occurred.
             */
            class HAZELCAST_API IOException : public IException {
            public:
                /**
                 * Constructor
                 */
                IOException(const std::string &source, const std::string &message);

                /**
                 * Destructor
                 */
                ~IOException() throw();

                /**
                 * return exception explanation string.
                 */
                char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_HazelcastIOException
