////
//// Created by msk on 3/13/13.
////
//// To change the template use AppCode | Preferences | File Templates.
////
//
//
//
//#ifndef HAZELCAST_MAP_PUT_OPERATION
//#define HAZELCAST_MAP_PUT_OPERATION
//
//#include <iostream>
//#include "PortableWriter.h"
//#include "PortableReader.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace serialization {
//
//            class MapPutOperation : public Portable {
//            public:
//
//                MapPutOperation();
//
//                MapPutOperation(Data key, Data value);
//
//                int getClassId();
//
//                void writePortable(PortableWriter & writer);
//
//                void readPortable(PortableReader & reader);
//                virtual int getFactoryId();
//
//                ~MapPutOperation();
//
//            private:
//                Data key;
//                Data value;
//
//
//            };
//        }
//    }
//}
//#endif //HAZELCAST_MAP_PUT_OPERATION
