//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/protocol/Principal.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace protocol {


            Principal::Principal() {

            }

            int Principal::getFactoryId() const {
                return ProtocolConstants::CLIENT_PORTABLE_FACTORY;;
            }

            int Principal::getClassId() const {
                return ProtocolConstants::PRINCIPAL_ID;;
            }

            void Principal::writePortable(serialization::PortableWriter &writer) const {
                writer.writeUTF("uuid", uuid);
                writer.writeUTF("ownerUuid", ownerUuid);
            };

            void Principal::readPortable(serialization::PortableReader& reader) {
                uuid = reader.readUTF("uuid");
                ownerUuid = reader.readUTF("ownerUuid");
            };


        }
    }
}
