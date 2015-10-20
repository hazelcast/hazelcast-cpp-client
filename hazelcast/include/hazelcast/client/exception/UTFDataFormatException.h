//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_UTFDATAFORMATEXCEPTION
#define HAZELCAST_UTFDATAFORMATEXCEPTION

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Raised the string format is not correct according to UTF-8
             */
            class HAZELCAST_API UTFDataFormatException : public IException {
            public:
                /**
                 * Constructor
                 */
                UTFDataFormatException(const std::string &source, const std::string &message);

                /**
                 * Destructor
                 */
                ~UTFDataFormatException() throw();

                /**
                 * return exception explanation string.
                 */
                char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_UTFDATAFORMATEXCEPTION

