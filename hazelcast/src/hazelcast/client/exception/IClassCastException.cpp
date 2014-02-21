//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/client/exception/IClassCastException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            IClassCastException::IClassCastException(const std::string &source, const std::string &message)
            : IException(source, message) {

            }

            IClassCastException::~IClassCastException() throw() {

            }

            char const *IClassCastException::what() const throw() {
                return IException::what();
            }
        }

    }
}
