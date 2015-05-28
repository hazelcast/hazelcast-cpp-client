//
// Created by sancar koyunlu on 23/07/14.
//


#include "hazelcast/client/exception/IllegalArgumentException.h"


namespace hazelcast {
    namespace client {
        namespace exception {

            IllegalArgumentException::IllegalArgumentException(const std::string& source, const std::string& message)
                    : IException(source, message) {
            }

            IllegalArgumentException::IllegalArgumentException(const char *source, const char *message)
                    : IException(source, message) {
            }

            IllegalArgumentException::~IllegalArgumentException() throw() {

            }

            char const *IllegalArgumentException::what() const throw() {
                return IException::what();
            }
        }
    }
}

