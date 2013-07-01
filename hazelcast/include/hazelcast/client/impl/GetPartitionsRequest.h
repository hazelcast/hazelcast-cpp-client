//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GET_PARTITIONS_REQUEST
#define HAZELCAST_GET_PARTITIONS_REQUEST

#include "../protocol/ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class GetPartitionsRequest {
            public:
                GetPartitionsRequest();
            };
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getSerializerId(const impl::GetPartitionsRequest& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getFactoryId(const impl::GetPartitionsRequest& ar) {
                return protocol::ProtocolConstants::PARTITION_DS_FACTORY;
            }

            inline int getClassId(const impl::GetPartitionsRequest& ar) {
                return protocol::ProtocolConstants::GET_PARTITIONS;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const impl::GetPartitionsRequest& arr) {
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, impl::GetPartitionsRequest& arr) {
            };

        }
    }
}
#endif //HAZELCAST_GET_PARTITIONS_REQUEST
