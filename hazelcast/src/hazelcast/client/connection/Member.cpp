//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/Member.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {

        bool Member::operator ==(const Member &rhs) const {
            return address == rhs.address;
        };

        int Member::operator <(const Member &rhs) const {
            return address < rhs.address;
        };

        const Address &Member::getAddress() const {
            return address;
        };

        const std::string &Member::getUuid() const {
            return uuid;
        };

        int Member::getFactoryId() const {
            return protocol::ProtocolConstants::DATA_FACTORY_ID;
        };

        int Member::getClassId() const {
            return protocol::ProtocolConstants::MEMBER_ID;
        };


        void Member::writeData(serialization::ObjectDataOutput &writer) const {
            address.writeData(writer);
            writer.writeUTF(uuid);
            int size = boolAttributes.size();
            size += byteAttributes.size();
            size += intAttributes.size();
            size += floatAttributes.size();
            size += shortAttributes.size();
            size += longAttributes.size();
            size += doubleAttributes.size();
            size += stringAttributes.size();
            writer.writeInt(size);

            for (std::map< std::string, bool >::const_iterator it = boolAttributes.begin(); it != boolAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_BOOLEAN);
                writer.writeBoolean(it->second);
            }
            for (std::map< std::string, byte >::const_iterator it = byteAttributes.begin(); it != byteAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_BYTE);
                writer.writeByte(it->second);
            }
            for (std::map< std::string, int >::const_iterator it = intAttributes.begin(); it != intAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_INTEGER);
                writer.writeInt(it->second);
            }
            for (std::map< std::string, float >::const_iterator it = floatAttributes.begin(); it != floatAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_FLOAT);
                writer.writeFloat(it->second);
            }
            for (std::map< std::string, short >::const_iterator it = shortAttributes.begin(); it != shortAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_SHORT);
                writer.writeShort(it->second);
            }
            for (std::map< std::string, long >::const_iterator it = longAttributes.begin(); it != longAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_LONG);
                writer.writeLong(it->second);
            }
            for (std::map< std::string, double >::const_iterator it = doubleAttributes.begin(); it != doubleAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_DOUBLE);
                writer.writeDouble(it->second);
            }
            for (std::map< std::string, std::string >::const_iterator it = stringAttributes.begin(); it != stringAttributes.end(); ++it) {
                writer.writeUTF(it->first);
                writer.writeByte(util::IOUtil::PRIMITIVE_TYPE_UTF);
                writer.writeUTF(it->second);
            }

        };

        void Member::readData(serialization::ObjectDataInput &reader) {
            address.readData(reader);
            uuid = reader.readUTF();
            int size = reader.readInt();
            for (int i = 0; i < size; i++) {
                std::string key = reader.readUTF();
                byte readByte = reader.readByte();
                if (readByte == util::IOUtil::PRIMITIVE_TYPE_BOOLEAN) {
                    boolAttributes[key] = reader.readBoolean();
                } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_BYTE) {
                    byteAttributes[key] = reader.readByte();
                } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_DOUBLE) {
                    doubleAttributes[key] = reader.readDouble();
                } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_FLOAT) {
                    floatAttributes[key] = reader.readFloat();
                } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_INTEGER) {
                    intAttributes[key] = reader.readInt();
                } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_LONG) {
                    longAttributes[key] = reader.readLong();
                } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_SHORT) {
                    shortAttributes[key] = reader.readShort();
                } else if (readByte == util::IOUtil::PRIMITIVE_TYPE_UTF) {
                    stringAttributes[key] = reader.readUTF();
                }
            }
        };


        std::string Member::getAttributeResolved(const std::string &key, std::string *tag) {
            return stringAttributes[key];
        }

        bool Member::getAttributeResolved(const std::string &key, bool *tag) {
            return boolAttributes[key];
        }

        byte Member::getAttributeResolved(const std::string &key, byte *tag) {
            return byteAttributes[key];
        }

        short Member::getAttributeResolved(const std::string &key, short *tag) {
            return shortAttributes[key];
        }

        int Member::getAttributeResolved(const std::string &key, int *tag) {
            return intAttributes[key];
        }

        long Member::getAttributeResolved(const std::string &key, long *tag) {
            return longAttributes[key];
        }

        float Member::getAttributeResolved(const std::string &key, float *tag) {
            return floatAttributes[key];
        }

        double Member::getAttributeResolved(const std::string &key, double *tag) {
            return doubleAttributes[key];
        }

        void Member::setAttributeResolved(const std::string &key, std::string value) {
            stringAttributes[key] = value;
        }

        void Member::setAttributeResolved(const std::string &key, bool value) {
            boolAttributes[key] = value;
        }

        void Member::setAttributeResolved(const std::string &key, byte value) {
            byteAttributes[key] = value;
        }

        void Member::setAttributeResolved(const std::string &key, short value) {
            shortAttributes[key] = value;
        }

        void Member::setAttributeResolved(const std::string &key, int value) {
            intAttributes[key] = value;
        }

        void Member::setAttributeResolved(const std::string &key, long value) {
            longAttributes[key] = value;
        }

        void Member::setAttributeResolved(const std::string &key, float value) {
            floatAttributes[key] = value;
        }

        void Member::setAttributeResolved(const std::string &key, double value) {
            doubleAttributes[key] = value;
        }

        bool Member::removeAttributeResolved(const std::string &key, std::string *tag) {
            return stringAttributes.erase(key) == 1;
        }

        bool Member::removeAttributeResolved(const std::string &key, bool *tag) {
            return boolAttributes.erase(key) == 1;
        }

        bool Member::removeAttributeResolved(const std::string &key, byte *tag) {
            return byteAttributes.erase(key) == 1;
        }

        bool Member::removeAttributeResolved(const std::string &key, short *tag) {
            return shortAttributes.erase(key) == 1;
        }

        bool Member::removeAttributeResolved(const std::string &key, int *tag) {
            return intAttributes.erase(key) == 1;
        }

        bool Member::removeAttributeResolved(const std::string &key, long *tag) {
            return longAttributes.erase(key) == 1;
        }

        bool Member::removeAttributeResolved(const std::string &key, float *tag) {
            return floatAttributes.erase(key) == 1;
        }

        bool Member::removeAttributeResolved(const std::string &key, double *tag) {
            return doubleAttributes.erase(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, std::string *tag) const {
            return stringAttributes.count(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, bool *tag) const {
            return boolAttributes.count(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, byte *tag) const {
            return byteAttributes.count(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, short *tag) const {
            return shortAttributes.count(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, int *tag) const {
            return intAttributes.count(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, long *tag) const {
            return longAttributes.count(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, float *tag) const {
            return floatAttributes.count(key) == 1;
        }

        bool Member::lookupAttributeResolved(const std::string &key, double *tag) const {
            return doubleAttributes.count(key) == 1;
        }
    }
}
