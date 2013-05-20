//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_PRINCIPAL
#define HAZELCAST_PRINCIPAL

#include <string>
#include "Portable.h"
#include "ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class Principal : public hazelcast::client::serialization::Portable {
            public:
                Principal();

                Principal(std::string uuid, std::string ownerUuid);

                std::string uuid;
                std::string ownerUuid;
            };
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getFactoryId(const hazelcast::client::protocol::Principal& ar) {
                return hazelcast::client::protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;;
            }

            inline int getClassId(const hazelcast::client::protocol::Principal& ar) {
                return hazelcast::client::protocol::ProtocolConstants::PRINCIPAL_ID;;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::protocol::Principal& data) {
                writer ["uuid"] << data.uuid;
                writer ["ownerUuid"] << data.ownerUuid;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::protocol::Principal& data) {
                reader ["uuid"] >> data.uuid;
                reader ["ownerUuid"] >> data.ownerUuid;
            };

        }
    }
}

#endif //HAZELCAST_PRINCIPAL
