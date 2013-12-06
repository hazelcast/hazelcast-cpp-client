//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapRemoveRequest
#define HAZELCAST_TxnMultiMapRemoveRequest

#include "hazelcast/client/multimap/TxnMultiMapRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class HAZELCAST_API TxnMultiMapRemoveRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key);

                TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key, const serialization::Data& value);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const serialization::Data& key;
                const serialization::Data *value;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapRemoveRequest
