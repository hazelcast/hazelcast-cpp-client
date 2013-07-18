//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//

#include "IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {

            IException::IException(const std::string& source, const std::string& message)
            : std::exception()
            , message(message)
            , source(source) {
            }

            IException::~IException() throw() {

            }

            char const *IException::what() const throw() {
                std::string report = "Source { " + source + "} , Error {" + message + "}";
                return report.c_str();
            }

        }
    }
}