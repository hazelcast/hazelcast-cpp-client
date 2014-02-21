//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_EXCEPTION
#define HAZELCAST_EXCEPTION

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <stdexcept>

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Base class for all exception originated from Hazelcast methods.
             * If exception coming from hazelcast servers cannot be identified,
             * it will be fired as IException.
             *
             *
             * @see InstanceNotActiveException
             * @see InterruptedException
             * @see IOException
             * @see IClassCastException
             * @see IllegalStateException
             */
            class HAZELCAST_API IException : public std::exception {
            public:
                /**
                 * Constructor
                 */
                IException();

                /**
                 * Constructor
                 */
                IException(const std::string &source, const std::string &message);

                /**
                 * Destructor
                 */
                virtual ~IException() throw();

                /**
                 * return exception explanation string.
                 */
                virtual char const *what() const throw();

            private:
                std::string report;
            };
        }
    }
}

#endif //HAZELCAST_EXCEPTION

