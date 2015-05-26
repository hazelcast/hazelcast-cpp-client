//
// Created by sancar koyunlu on 23/07/14.
//


#ifndef HAZELCAST_IllegalArgumentException
#define HAZELCAST_IllegalArgumentException

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
            * Raised when an illegal argument is passed to Hazelcast API
            */
            class HAZELCAST_API IllegalArgumentException : public IException {
            public:
                /**
                * Constructor
                */
                IllegalArgumentException(const std::string& source, const std::string& message);

                IllegalArgumentException(const char *source, const char *message);

                /**
                * Destructor
                */
                ~IllegalArgumentException() throw();

                /**
                * return exception explanation string.
                */
                char const *what() const throw();
            };
        }
    }
}

#endif //HAZELCAST_IllegalArgumentException
