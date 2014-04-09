//
// Created by sancar koyunlu on 20/02/14.
//


#ifndef HAZELCAST_ICastException
#define HAZELCAST_ICastException

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Raised an unexpected type data comes from server.
             */
            class HAZELCAST_API IClassCastException : public IException {
            public:
                /**
                 * Constructor
                 */
                IClassCastException(const std::string &source, const std::string &message);

                /**
                 * Destructor
                 */
                ~IClassCastException() throw();

                /**
                 * return exception explanation string.
                 */
                char const *what() const throw();
            };
        }
    }
}

#endif //HAZELCAST_ICastException

