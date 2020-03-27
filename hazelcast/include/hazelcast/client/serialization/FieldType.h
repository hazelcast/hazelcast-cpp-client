/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAZELCAST_FIELD_TYPE
#define HAZELCAST_FIELD_TYPE

#include <ostream>
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class HAZELCAST_API FieldType {
            public:
                FieldType();

                FieldType(int type);

                FieldType(FieldType const& rhs);

                const byte getId() const;

                FieldType& operator=(FieldType const& rhs);

                bool operator==(FieldType const& rhs) const;

                bool operator!=(FieldType const& rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const FieldType &type);

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
                static const FieldType TYPE_BOOLEAN_ARRAY(12);
                static const FieldType TYPE_CHAR_ARRAY(13);
                static const FieldType TYPE_SHORT_ARRAY(14);
                static const FieldType TYPE_INT_ARRAY(15);
                static const FieldType TYPE_LONG_ARRAY(16);
                static const FieldType TYPE_FLOAT_ARRAY(17);
                static const FieldType TYPE_DOUBLE_ARRAY(18);
                static const FieldType TYPE_UTF_ARRAY(19);
            }
        }
    }
}
#endif /* HAZELCAST_FIELD_TYPE */

