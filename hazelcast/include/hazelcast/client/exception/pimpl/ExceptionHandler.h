//
// Created by sancar koyunlu on 04/04/14.
//


#ifndef HAZELCAST_ExceptionHandler
#define HAZELCAST_ExceptionHandler

#include "hazelcast/util/HazelcastDll.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace exception {
            namespace pimpl{
                class HAZELCAST_API ExceptionHandler {
                public:
                    /**
                    * InternalAPI java side class name of interrupted exception
                    */
                    static std::string INTERRUPTED;
                    /**
                    * InternalAPI java side class name of instance not active exception
                    */

                    static std::string INSTANCE_NOT_ACTIVE;
                    /**
                    * InternalAPI rethrows the exception with appropriate type
                    *
                    * @param exceptionName java(node) side exceptionName
                    * @param message exception cause message
                    */
                    static void rethrow(const std::string &exceptionName, const std::string &message);

                };
            }
        }
    }
}

#endif //HAZELCAST_ExceptionHandler

