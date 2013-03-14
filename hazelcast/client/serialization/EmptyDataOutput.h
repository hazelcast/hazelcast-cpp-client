//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_EMPTY_DATA_OUTPUT
#define HAZELCAST_EMPTY_DATA_OUTPUT

#include <iostream>
#include "DataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {


            class EmptyDataOutput : public DataOutput{

            public:

                EmptyDataOutput();

                virtual std::vector<unsigned char> toByteArray();

                virtual std::string toString();

                virtual int getSize();

                virtual void write(const std::vector<unsigned char>  & bytes);

                virtual void write(char const *bytes, int length);

                virtual void writeBoolean(bool b);

                virtual void writeByte(int i);

                virtual void writeShort(int i);

                virtual void writeChar(int i);

                virtual void writeInt(int i);

                virtual void writeLong(long l);

                virtual void writeFloat(float v);

                virtual void writeDouble(double v);

                virtual void writeUTF(std::string s);

                virtual void write(int index, int b);

                virtual void write(int index, char *b, int off, int len);

                virtual void writeInt(int index, int v);

                virtual void writeLong(int index, long const v);

                virtual void writeBoolean(int index, bool v);

                virtual void writeByte(int index, int const v);

                virtual void writeChar(int index, int const v);

                virtual void writeDouble(int index, double const v);

                virtual void writeFloat(int index, float const v);

                virtual void writeShort(int index, int const v);

                virtual int position();

                virtual void position(int newPos);

                virtual void reset();





            };

        }
    }
}
#endif //HAZELCAST_EMPTY_DATA_OUTPUT
