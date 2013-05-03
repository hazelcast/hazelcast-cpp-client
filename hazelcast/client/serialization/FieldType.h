#ifndef HAZELCAST_FIELD_TYPE
#define HAZELCAST_FIELD_TYPE

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

//            enum FieldTypes {
//                PORTABLE,
//                BYTE,
//                BOOLEAN,
//                CHAR,
//                SHORT,
//                INT,
//                LONG,
//                FLOAT,
//                DOUBLE,
//                UTF,
//                PORTABLE_ARRAY,
//                BYTE_ARRAY,
//                CHAR_ARRAY,
//                SHORT_ARRAY,
//                INT_ARRAY,
//                LONG_ARRAY,
//                FLOAT_ARRAY,
//                DOUBLE_ARRAY
//            };
            class FieldType {
            public:

                FieldType(int type) :type((byte) type) {
                };

                FieldType(FieldType const& rhs) : type(rhs.type) {

                };

                const byte getId() const {
                    return type;
                };

                FieldType& operator = (FieldType const& rhs) {
                    this->type = rhs.type;
                    return (*this);
                };

                bool operator ==(FieldType const& rhs) const {
                    if (type != rhs.type) return false;
                    return true;
                };

                bool operator !=(FieldType const& rhs) const {
                    if (type == rhs.type) return false;
                    return true;
                };

            private:
                byte type;
            };

            namespace FieldTypes {
                static const FieldType TYPE_PORTABLE(0);
                static const FieldType TYPE_BYTE(1);
                static const FieldType TYPE_BOOLEAN(2);
                static const FieldType TYPE_CHAR(3);
                static const FieldType TYPE_SHORT(4);
                static const FieldType TYPE_INT(5);
                static const FieldType TYPE_LONG(6);
                static const FieldType TYPE_FLOAT(7);
                static const FieldType TYPE_DOUBLE(8);
                static const FieldType TYPE_UTF(9);
                static const FieldType TYPE_PORTABLE_ARRAY(10);
                static const FieldType TYPE_BYTE_ARRAY(11);
                static const FieldType TYPE_CHAR_ARRAY(12);
                static const FieldType TYPE_SHORT_ARRAY(13);
                static const FieldType TYPE_INT_ARRAY(14);
                static const FieldType TYPE_LONG_ARRAY(15);
                static const FieldType TYPE_FLOAT_ARRAY(16);
                static const FieldType TYPE_DOUBLE_ARRAY(17);
            }
        }
    }
}
#endif /* HAZELCAST_FIELD_TYPE */
