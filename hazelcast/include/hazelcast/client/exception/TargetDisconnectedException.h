//
// Created by sancar koyunlu on 03/01/14.
//


#ifndef HAZELCAST_TargetDisconnectedException
#define HAZELCAST_TargetDisconnectedException

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            class HAZELCAST_API TargetDisconnectedException : public IException {
            public:
                TargetDisconnectedException(const Address& address);

                virtual ~TargetDisconnectedException() throw();

                virtual char const *what() const throw();

            private:
                Address address;
            };
        }
    }
}


#endif //HAZELCAST_TargetDisconnectedException
