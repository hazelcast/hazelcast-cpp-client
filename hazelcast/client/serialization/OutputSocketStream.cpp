#include "OutputSocketStream.h"
#include "HazelcastException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            OutputSocketStream::OutputSocketStream(hazelcast::client::protocol::Socket & socket):socket(socket) {
            }

            OutputSocketStream::~OutputSocketStream() {
            }

            void OutputSocketStream::put(char c) {
                socket.sendData((void *) &c, 1);
            }

            void OutputSocketStream::put(int n, char c) {
                throw hazelcast::client::HazelcastException("Unsupported operation");
            }

            void OutputSocketStream::write(char const *buffer, int size) {
                socket.sendData((void *) buffer, size);
            }

            std::vector<byte>  OutputSocketStream::toByteArray() {
                throw hazelcast::client::HazelcastException("Unsupported operation");
                return std::vector<unsigned char>();
            }

            void OutputSocketStream::reset() {
                throw hazelcast::client::HazelcastException("Unsupported operation");
            }

            void OutputSocketStream::resize(int size) {
                throw hazelcast::client::HazelcastException("Unsupported operation");
            }

            int OutputSocketStream::size() {
                throw hazelcast::client::HazelcastException("Unsupported operation");
                return 0;
            }

        }
    }
}