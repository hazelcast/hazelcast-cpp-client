//
// Created by sancar koyunlu on 21/02/14.
//


#ifndef HAZELCAST_IAuthenticationException
#define HAZELCAST_IAuthenticationException


#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Raised an unexpected type data comes from server.
             */
            class HAZELCAST_API IAuthenticationException : public IException {
            public:
                /**
                 * Constructor
                 */
                IAuthenticationException(const std::string &source, const std::string &message);

                /**
                 * Destructor
                 */
                ~IAuthenticationException() throw();

                /**
                 * return exception explanation string.
                 */
                char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_IAuthenticationException

