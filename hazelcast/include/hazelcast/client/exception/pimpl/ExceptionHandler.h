//
// Created by sancar koyunlu on 04/04/14.
//


#ifndef HAZELCAST_ExceptionHandler
#define HAZELCAST_ExceptionHandler

#include "hazelcast/util/HazelcastDll.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
            namespace pimpl{
                class ExceptionHandler {
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ExceptionHandler

