//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnSizeRequest
#define HAZELCAST_TxnSizeRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API TxnSizeRequest : public impl::PortableRequest {
            public:
                TxnSizeRequest(const std::string &name);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                const std::string& name;
            };
        }
    }
}


#endif //HAZELCAST_TxnSizeRequest
