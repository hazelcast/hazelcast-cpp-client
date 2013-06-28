//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//

#include "HazelcastException.h"

namespace hazelcast {
    namespace client {

        HazelcastException::HazelcastException(std::string message)
        : std::domain_error(message) {
        }

        HazelcastException::~HazelcastException() throw() {

        }

        char const *HazelcastException::what() const throw() {
            return std::domain_error::what();
        }

    }
}