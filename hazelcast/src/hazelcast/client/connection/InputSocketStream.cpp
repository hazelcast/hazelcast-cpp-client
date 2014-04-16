//
// Created by sancar koyunlu on 5/10/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/connection/InputSocketStream.h"
#include "hazelcast/client/serialization/pimpl/SerializationContext.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            InputSocketStream::InputSocketStream(Socket &socket)
            :socket(socket)
            , context(NULL) {
            };

            void InputSocketStream::setSerializationContext(serialization::pimpl::SerializationContext *context) {
                this->context = context;
            };

            void InputSocketStream::readFully(std::vector<byte> &bytes) {
                socket.receive(&(bytes[0]), bytes.size(), MSG_WAITALL);
            };

            int InputSocketStream::skipBytes(int i) {
                std::vector<byte> temp(i);
                socket.receive((void *) &(temp[0]), i, MSG_WAITALL);
                return i;
            };

            int InputSocketStream::readInt() {
                byte s[4];
                socket.receive(s, sizeof(byte) * 4, MSG_WAITALL);
                return (0xff000000 & (s[0] << 24)) |
                        (0x00ff0000 & (s[1] << 16)) |
                        (0x0000ff00 & (s[2] << 8)) |
                        (0x000000ff & s[3]);
            };


            void InputSocketStream::readData(serialization::pimpl::Data &data) {
                data.setType(readInt());
                int classId = readInt();

                if (classId != data.NO_CLASS_ID) {
                    int factoryId = readInt();
                    int version = readInt();

                    int classDefSize = readInt();
                    if (context->isClassDefinitionExists(factoryId, classId, version)) {
                        data.cd = context->lookup(factoryId, classId, version);
                        skipBytes(classDefSize);
                    } else {
                        std::auto_ptr< std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                        readFully(*(classDefBytes.get()));
                        data.cd = context->createClassDefinition(factoryId, classDefBytes);
                    }
                }
                int size = readInt();
                if (size > 0) {
                    data.buffer->resize(size);
                    readFully(*(data.buffer.get()));
                }
                data.setPartitionHash(readInt());
            }
        }
    }
}

