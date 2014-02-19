//
// Created by sancar koyunlu on 5/10/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/serialization/InputSocketStream.h"
#include "hazelcast/client/serialization/SerializationContext.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            InputSocketStream::InputSocketStream(Socket &socket)
            :socket(socket)
            , context(NULL) {
            };

            void InputSocketStream::setSerializationContext(SerializationContext *context) {
                this->context = context;
            };

            SerializationContext *InputSocketStream::getSerializationContext() {
                assert(context != NULL);
                return context;
            };

            void InputSocketStream::readFully(std::vector<byte> &bytes) {
                socket.receive(bytes.data(), bytes.size(), MSG_WAITALL);
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

        }
    }
}
