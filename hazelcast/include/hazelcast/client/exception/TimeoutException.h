//
// Created by sancar koyunlu on 21/08/14.
//


#ifndef HAZELCAST_ITimeoutException
#define HAZELCAST_ITimeoutException

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
            * Raised when an operation is timed out
            */
            class TimeoutException : public IException {

            public:
                /**
                * Constructor
                */
                TimeoutException(const std::string& source, const std::string& message);

                /**
                * Destructor
                */
                ~TimeoutException() throw();

                /**
                * return exception explanation string.
                */
                char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_ITimeoutException
