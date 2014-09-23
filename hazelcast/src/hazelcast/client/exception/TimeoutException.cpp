//
// Created by sancar koyunlu on 21/08/14.
//

#include "hazelcast/client/exception/TimeoutException.h"


namespace hazelcast {
    namespace client {
        namespace exception {
            TimeoutException::TimeoutException(const std::string &source, const std::string &message)
            : IException(source, message) {

            }

            TimeoutException::~TimeoutException() throw(){

            }

            char const *TimeoutException::what() const throw() {
                return IException::what();
            }
        }

    }
}

