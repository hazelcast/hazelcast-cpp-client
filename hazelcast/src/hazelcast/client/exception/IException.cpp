//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            IException::IException()
            : std::exception()
            , report("") {

            }


            IException::IException(const std::string& source, const std::string& message)
            : std::exception() {
                report = "ExceptionMessage {" + message + "} at " + source;
            }

            IException::~IException() throw() {

            }

            char const *IException::what() const throw() {
                return report.c_str();
            }

        }
    }
}
