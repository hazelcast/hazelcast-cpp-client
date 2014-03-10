//
// Created by sancar koyunlu on 23/01/14.
//


#ifndef HAZELCAST_Credentials
#define HAZELCAST_Credentials

#include "hazelcast/client/serialization/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        /**
         * Credentials is a container object for endpoint (Members and Clients)
         * security attributes.
         *
         * It is used on authentication process by LoginModule.
         */
        class HAZELCAST_API Credentials : public serialization::Portable {
        public:
            /**
             * Returns IP address of endpoint.
             *
             * @return endpoint address
             */
            virtual const std::string &getEndpoint() const = 0;

            /**
             * Sets IP address of endpoint.
             *
             * @param endpoint address
             */
            virtual void setEndpoint(const std::string &endpoint) = 0;

            /**
             * Returns principal of endpoint.
             *
             * @return endpoint principal
             */
            virtual const std::string &getPrincipal() const = 0;

            /**
             *  Destructor
             */
            virtual ~Credentials() {

            }

        };
    }
}


#endif //HAZELCAST_Credentials
