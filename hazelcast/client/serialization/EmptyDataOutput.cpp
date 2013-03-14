//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "EmptyDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            EmptyDataOutput::EmptyDataOutput() : DataOutput(NULL,NULL) {
            }

            std::vector<unsigned char>  EmptyDataOutput::toByteArray() {

            }

            std::string EmptyDataOutput::toString() {
                return "";
            }

            int EmptyDataOutput::getSize() {
                return 0;
            }

            void EmptyDataOutput::write(const std::vector<unsigned char>  & bytes) {

            }

            void EmptyDataOutput::write(char const *bytes, int length) {

            }

            void EmptyDataOutput::writeBoolean(bool b) {

            }

            void EmptyDataOutput::writeByte(int i) {

            }

            void EmptyDataOutput::writeShort(int i) {

            }

            void EmptyDataOutput::writeChar(int i) {

            }

            void EmptyDataOutput::writeInt(int i) {

            }

            void EmptyDataOutput::writeLong(long l) {

            }

            void EmptyDataOutput::writeFloat(float v) {

            }

            void EmptyDataOutput::writeDouble(double v) {

            }

            void EmptyDataOutput::writeUTF(std::string s) {

            }

            void EmptyDataOutput::write(int index, int b) {

            }

            void EmptyDataOutput::write(int index, char *b, int off, int len) {

            }

            void EmptyDataOutput::writeInt(int index, int v) {

            }

            void EmptyDataOutput::writeLong(int index, long const v) {

            }

            void EmptyDataOutput::writeBoolean(int index, bool v) {

            }

            void EmptyDataOutput::writeByte(int index, int const v) {

            }

            void EmptyDataOutput::writeChar(int index, int const v) {

            }

            void EmptyDataOutput::writeDouble(int index, double const v) {

            }

            void EmptyDataOutput::writeFloat(int index, float const v) {

            }

            void EmptyDataOutput::writeShort(int index, int const v) {

            }

            int EmptyDataOutput::position() {
                return 0;
            }

            void EmptyDataOutput::position(int newPos) {

            }

            void EmptyDataOutput::reset() {

            }


        }
    }
}