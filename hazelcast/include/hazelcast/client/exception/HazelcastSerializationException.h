//
// Created by sancar koyunlu on 23/07/14.
//


#ifndef HAZELCAST_ISerializationException
#define HAZELCAST_ISerializationException


#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
            * Raised when an error occur during serialization or deserialization in Hazelcast
            */
            class HAZELCAST_API HazelcastSerializationException : public IException {
            public:
                /**
                * Constructor
                */
                HazelcastSerializationException(const std::string& source, const std::string& message);

                /**
                * Destructor
                */
                ~HazelcastSerializationException() throw();

                /**
                * return exception explanation string.
                */
                char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_ISerializationException
