////
//// Created by sancar koyunlu on 6/7/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//
//#ifndef HAZELCAST_TYPE_SERIALIZER
//#define HAZELCAST_TYPE_SERIALIZER
//
//namespace hazelcast {
//    namespace client {
//        namespace serialization {
//            template <typename CustomSerializer>
//            class TypeSerializer {
//            public:
//                virtual ~TypeSerializer() {
//
//                };
//
////                template <typename CustomSerializer>
////                TypeSerializer(CustomSerializer* customSerializer){
////                      holder = new PlaceHolder(customSerializer);
////                }
////
////                template <typename T>
////                void write(BufferedDataOutput& out, const T& object) {
////                    dynamic_cast<Holder<?>*>(holder)->value->write(out, object);
//////                    static_cast<CustomSerializer *>(this)->write(out, object);
////                };
////
////                template <typename T>
////                T read(BufferedDataInput& in, const Data& data) {
////                    static_cast<CustomSerializer *>(this)->read(in, data);
////                };
////
////                virtual int getTypeId(){
////                    static_cast<CustomSerializer *>(this)->read(in, data);
////                };
//                private:
////                    PlaceHolder* holder;
//            };
//
////            class PlaceHolder {
////            public:
////                virtual ~PlaceHolder() {
////
////                };
////            };
////
////            template <typename ValueType>
////            class Holder : public PlaceHolder {
////            public:
////                Holder(ValueType *value) {
////                    this->value = value;
////                }
////
////                ValueType *value;
////            private:
////
////                ~Holder(){
////
////                };
////            };
//
//        }
//    }
//}
//
//
//#endif //HAZELCAST_TYPE_SERIALIZER
