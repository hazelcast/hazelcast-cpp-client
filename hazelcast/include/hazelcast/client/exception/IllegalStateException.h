//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_HazelcastIllegalStateException
#define HAZELCAST_HazelcastIllegalStateException

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Raised when method is called in a illegal state.
             */
            class HAZELCAST_API IllegalStateException : public IException {
            public:
                /**
                 * Constructor
                 */
                IllegalStateException(const std::string& source, const std::string& message);
                /**
                 * Destructor
                 */
                virtual ~IllegalStateException() throw();
                /**
                 * return exception explanation string.
                 */
                virtual char const *what() const throw();
            };
        }
    }
}

#endif //HAZELCAST_HazelcastIllegalStateException

