//
// Created by sancar koyunlu on 07/04/14.
//

#include "hazelcast/client/impl/ServerException.h"
#include "hazelcast/client/exception/InterruptedException.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/client/exception/pimpl/ExceptionHandler.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            namespace pimpl{
                std::string ExceptionHandler::INTERRUPTED = "InterruptedException";
                std::string ExceptionHandler::INSTANCE_NOT_ACTIVE = "HazelcastInstanceNotActiveException";

                void ExceptionHandler::rethrow(const std::string &exceptionName, const std::string &message) {
                    if (INTERRUPTED == exceptionName) {
                        exception::InterruptedException exception(exceptionName, message);
                        throw  exception;
                    } else if (INSTANCE_NOT_ACTIVE == exceptionName) {
                        exception::InstanceNotActiveException exception(message);
                        throw  exception;
                    } else {
                        exception::IException exception("Server:", message);
                        throw  exception;
                    }
                }
            }
        }
    }
}
