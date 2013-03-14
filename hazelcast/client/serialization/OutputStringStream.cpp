//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//

#include "OutputStringStream.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            OutputStringStream::OutputStringStream() {
            };

            OutputStringStream::~OutputStringStream() {

            };

            void OutputStringStream::put(char c) {
                outputStream.push_back(c);
            };

            void OutputStringStream::put(int n, char c) {
                outputStream[n] = c;
            };

            void OutputStringStream::write(char const *buffer, int size) {
                outputStream.insert(outputStream.end(), buffer, buffer + size);
            };

            std::vector<byte>  OutputStringStream::toByteArray() {
                return outputStream;
            };

            void OutputStringStream::reset() {
                outputStream.clear();
            };

            void OutputStringStream::resize(int size) {
                if (outputStream.size() < size)
                    outputStream.resize(size, 0);
            };

            int OutputStringStream::size() {
                return outputStream.size();
            };
        }
    }
}