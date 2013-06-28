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
//
//            class PrivateTypeSerializerBase {
//            public:
//                virtual ~PrivateTypeSerializerBase() {
//                };
//
//                virtual int getTypeId() const = 0;
//            };
//
//            template <typename CustomSerializer>
//            class TypeSerializer : public PrivateTypeSerializerBase {
//            public:
//                virtual ~TypeSerializer() {
//
//                };
//
//                template <typename CustomSerializer>
//                TypeSerializer(CustomSerializer *customSerializer) {
//                }
//
//                template <typename T>
//                void write(BufferedDataOutput& out, const T& object) {
//                    static_cast<CustomSerializer *>(this)->write(out, object);
//                };
//
//                template <typename T>
//                T read(BufferedDataInput& in, const Data& data) {
//                    static_cast<CustomSerializer *>(this)->read(in, data);
//                };
//
//            };
//
//        }
//    }
//}
//
//
//#endif //HAZELCAST_TYPE_SERIALIZER
