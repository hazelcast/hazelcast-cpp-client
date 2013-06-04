//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PARTITION_RESPONSE
#define HAZELCAST_PARTITION_RESPONSE

#include <vector>
#include "../Address.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class PartitionsResponse {
                template<typename HzWriter>
                friend void hazelcast::client::serialization::writePortable(HzWriter& writer, const hazelcast::client::impl::PartitionsResponse& arr);

                template<typename HzReader>
                friend void hazelcast::client::serialization::readPortable(HzReader& reader, hazelcast::client::impl::PartitionsResponse& arr);

            public:
                PartitionsResponse();

                PartitionsResponse(std::vector<Address>& addresses, std::vector<int>& ownerIndexes);

                std::vector<Address> getMembers() const;

                std::vector<int> getOwnerIndexes() const;

                bool isEmpty();

            private:
                std::vector<Address> members;
                std::vector<int> ownerIndexes;
            };

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getTypeId(const hazelcast::client::impl::PartitionsResponse& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getFactoryId(const hazelcast::client::impl::PartitionsResponse& ar) {
                return hazelcast::client::protocol::ProtocolConstants::PARTITION_DS_FACTORY;
            }

            inline int getClassId(const hazelcast::client::impl::PartitionsResponse& ar) {
                return hazelcast::client::protocol::ProtocolConstants::PARTITIONS;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::impl::PartitionsResponse& arr) {
                writer << arr.members.size();
                for (std::vector<Address>::const_iterator it = arr.members.begin(); it != arr.members.end(); ++it) {
                    writer << (*it);
                }
                writer << arr.ownerIndexes.size();
                for (std::vector<int>::const_iterator it = arr.ownerIndexes.begin(); it != arr.ownerIndexes.end(); ++it) {
                    writer << (*it);
                }
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::impl::PartitionsResponse& arr) {
                int len;
                reader >> len;
                arr.members.resize(len);
                for (int i = 0; i < len; i++) {
                    reader >> arr.members[i];
                }
                reader >> len;
                arr.ownerIndexes.resize(len);
                for (int i = 0; i < len; i++) {
                    reader >> arr.ownerIndexes[i];
                }
            };

        }
    }
}
#endif //HAZELCAST_PARTITION_RESPONSE
