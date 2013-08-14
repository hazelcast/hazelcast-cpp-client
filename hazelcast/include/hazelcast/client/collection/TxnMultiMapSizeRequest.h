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

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    TxnMultiMapRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    TxnMultiMapRequest::readPortable(reader);
                };
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapSizeRequest
