//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/client/exception/IAuthenticationException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            IAuthenticationException::IAuthenticationException(const std::string &source, const std::string &message)
            : IException(source, message) {

            }

            IAuthenticationException::~IAuthenticationException() throw() {

            }

            char const *IAuthenticationException::what() const throw() {
                return IException::what();
            }
        }

    }
}
