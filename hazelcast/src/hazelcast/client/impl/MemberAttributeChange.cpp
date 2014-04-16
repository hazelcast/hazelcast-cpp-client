//
// Created by sancar koyunlu on 23/01/14.
//

#include "hazelcast/client/impl/MemberAttributeChange.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            MemberAttributeChange::MemberAttributeChange() {

            }

            const std::string &MemberAttributeChange::getUuid() const {
                return uuid;
            }

            MemberAttributeEvent::MemberAttributeOperationType MemberAttributeChange::getOperationType() const {
                return operationType;
            }

            const std::string &MemberAttributeChange::getKey() const {
                return key;
            }

            const std::string &MemberAttributeChange::getValue() const {
                return value;
            }

            util::IOUtil::PRIMITIVE_ID MemberAttributeChange::getTypeId() const {
                return primitive_id;
            }

            void MemberAttributeChange::writeData(serialization::ObjectDataOutput &writer) const {
                assert(0 && "No one will call it");
            }

            void MemberAttributeChange::readData(serialization::ObjectDataInput &reader) {
                uuid = reader.readUTF();
                key = reader.readUTF();
                int operation = reader.readByte();
                if (operation == MemberAttributeEvent:: PUT) {
                    operationType = MemberAttributeEvent::PUT;
                    byte readByte = reader.readByte();
                    if (readByte == util::IOUtil::PRIMITIVE_TYPE_BOOLEAN) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_BOOLEAN;
                        value = util::IOUtil::to_string(reader.readBoolean());
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_BYTE) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_BYTE;
                        value = util::IOUtil::to_string(reader.readByte());
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_DOUBLE) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_DOUBLE;
                        value = util::IOUtil::to_string(reader.readDouble());
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_FLOAT) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_FLOAT;
                        value = util::IOUtil::to_string(reader.readFloat());
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_INTEGER) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_INTEGER;
                        value = util::IOUtil::to_string(reader.readInt());
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_LONG) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_LONG;
                        value = util::IOUtil::to_string(reader.readLong());
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_SHORT) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_SHORT;
                        value = util::IOUtil::to_string(reader.readShort());
                    } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_UTF) {
                        primitive_id = util::IOUtil::PRIMITIVE_TYPE_UTF;
                        value = reader.readUTF();
                    }
                } else if (operation == MemberAttributeEvent::REMOVE) {
                    primitive_id = util::IOUtil::PRIMITIVE_TYPE_NULL;
                    operationType = MemberAttributeEvent::REMOVE;
                }

            }
        }
    }
}
