#ifndef HAZELCAST_FIELD_TYPE
#define HAZELCAST_FIELD_TYPE

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class HAZELCAST_API FieldType {
            public:
                FieldType() : id(0) {

                };

                FieldType(int type) :id((byte) type) {
                };

                FieldType(FieldType const& rhs) : id(rhs.id) {

                };

                const byte getId() const {
                    return id;
                };

                FieldType& operator = (FieldType const& rhs) {
                    this->id = rhs.id;
                    return (*this);
                };

                bool operator ==(FieldType const& rhs) const {
                    if (id != rhs.id) return false;
                    return true;
                };

                bool operator !=(FieldType const& rhs) const {
                    if (id == rhs.id) return false;
                    return true;
                };

                byte id;
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
