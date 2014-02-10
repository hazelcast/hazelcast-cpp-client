//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_InterruptedException
#define HAZELCAST_InterruptedException

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Some of the Hazelcast operations may throw an InterruptedException
             * if a user thread is interrupted while waiting a response.
             *
             */
            class HAZELCAST_API InterruptedException : public IException {
            public:
                /**
                 * Constructor
                 */
                InterruptedException(const std::string& source, const std::string& message);
                /**
                 * Destructor
                 */
                virtual ~InterruptedException() throw();

                /**
                 * return exception explanation string.
                 */
                virtual char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_InterruptedException
