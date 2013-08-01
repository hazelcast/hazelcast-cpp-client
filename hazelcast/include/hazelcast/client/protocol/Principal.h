//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_PRINCIPAL
#define HAZELCAST_PRINCIPAL

#include "ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace protocol {
            class Principal : public Portable {
            public:
                Principal();

                Principal(std::string uuid, std::string ownerUuid);

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer.writeUTF("uuid", uuid);
                    writer.writeUTF("ownerUuid", ownerUuid);
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    uuid = reader.readUTF("uuid");
                    ownerUuid = reader.readUTF("ownerUuid");
                };
            private:
                std::string uuid;
                std::string ownerUuid;
            };
        }
    }
}
#endif //HAZELCAST_PRINCIPAL
