//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "MapPutOperation.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            MapPutOperation::MapPutOperation() {

            }

            MapPutOperation::MapPutOperation(Data key, Data value)
            :key(key), value(value) {

            }

            int MapPutOperation::getFactoryId() {
                return 0;
            }

            int MapPutOperation::getClassId() {
                return -1;
            }

            void MapPutOperation::writePortable(PortableWriter & writer) {
                DataOutput *output = writer.getRawDataOutput();
                key.writeData(*output);
                value.writeData(*output);
            }

            void MapPutOperation::readPortable(PortableReader & reader) {
                DataInput *input = reader.getRawDataInput();
                key.readData(*input);
                value.readData(*input);
            }

            MapPutOperation::~MapPutOperation() {


            }


        }
    }
}