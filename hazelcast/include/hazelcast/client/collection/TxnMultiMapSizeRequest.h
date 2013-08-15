//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapSizeRequest
#define HAZELCAST_TxnMultiMapSizeRequest

#include "TxnMultiMapRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnMultiMapSizeRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapSizeRequest(const std::string& name);

                int getClassId() const;


                void writePortable(serialization::PortableWriter& writer) const {
                    TxnMultiMapRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    TxnMultiMapRequest::readPortable(reader);
                };
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapSizeRequest
