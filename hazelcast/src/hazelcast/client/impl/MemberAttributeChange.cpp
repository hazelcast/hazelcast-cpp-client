//
// Created by sancar koyunlu on 23/01/14.
//

#include "hazelcast/client/impl/MemberAttributeChange.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            MemberAttributeChange::MemberAttributeChange() {

            }

            const std::string &MemberAttributeChange::getUuid() const {
                return uuid;
            }

            int MemberAttributeChange::getOperationType() const {
                return operationType;
            }

            const std::string &MemberAttributeChange::getKey() const {
                return key;
            }

            void MemberAttributeChange::writeData(serialization::ObjectDataOutput &writer) const {
                assert(0 && "No one will call it");
            }

            void MemberAttributeChange::readData(serialization::ObjectDataInput &reader) {
                uuid = reader.readUTF();
                key = reader.readUTF();
                int operation = reader.readByte();
                if (operation == DELTA_MEMBER_PROPERTIES_OP_PUT) {
                    operationType = DELTA_MEMBER_PROPERTIES_OP_PUT;
                    byte readByte = reader.readByte();
                    if (readByte == util::IOUtil::PRIMITIVE_TYPE_BOOLEAN) {
                        boolValue = reader.readBoolean();
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_BYTE) {
                        byteValue = reader.readByte();
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_DOUBLE) {
                        doubleValue = reader.readDouble();
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_FLOAT) {
                        floatValue = reader.readFloat();
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_INTEGER) {
                        intValue = reader.readInt();
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_LONG) {
                        longValue = reader.readLong();
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_SHORT) {
                        shortValue = reader.readShort();
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_UTF) {
                        stringValue = reader.readUTF();
                    }
                } else if (operation == DELTA_MEMBER_PROPERTIES_OP_REMOVE) {
                    operationType = DELTA_MEMBER_PROPERTIES_OP_REMOVE;
                }

            }
        }
    }
}