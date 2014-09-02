//
// Created by sancar koyunlu on 23/07/14.
//

#include "hazelcast/client/exception/HazelcastSerializationException.h"


namespace hazelcast {
    namespace client {
        namespace exception {

            HazelcastSerializationException::HazelcastSerializationException(const std::string& source, const std::string& message)
            : IException(source, message) {

            }

            HazelcastSerializationException::~HazelcastSerializationException() throw(){

            }

            char const *HazelcastSerializationException::what() const throw(){
                return IException::what();
            }
        }
    }
}

