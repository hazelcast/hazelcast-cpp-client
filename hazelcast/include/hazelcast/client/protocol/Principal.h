//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_PRINCIPAL
#define HAZELCAST_PRINCIPAL

#include "ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace protocol {
            class Principal {
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

            inline int getSerializerId(const protocol::Principal& x) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            inline int getFactoryId(const protocol::Principal& ar) {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;;
            }

            inline int getClassId(const protocol::Principal& ar) {
                return protocol::ProtocolConstants::PRINCIPAL_ID;;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const protocol::Principal& data) {
                writer ["uuid"] << data.uuid;
                writer ["ownerUuid"] << data.ownerUuid;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, protocol::Principal& data) {
                reader ["uuid"] >> data.uuid;
                reader ["ownerUuid"] >> data.ownerUuid;
            };

        }
    }
}

#endif //HAZELCAST_PRINCIPAL
