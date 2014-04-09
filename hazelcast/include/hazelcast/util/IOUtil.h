//
// Created by sancar koyunlu on 23/01/14.
//


#ifndef HAZELCAST_IOUtil
#define HAZELCAST_IOUtil

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <sstream>

namespace hazelcast {
    namespace util {
        class HAZELCAST_API IOUtil {
        public:
            template<typename T>
            static std::string to_string(T value) {
                std::stringstream s;
                s << value;
                return s.str();
            };

            template<typename T>
            static T to_value(const std::string &str) {
                std::stringstream s(str);
                T value;
                s >> value;
                return value;
            };

            enum PRIMITIVE_ID {
                PRIMITIVE_TYPE_BOOLEAN = 1,
                PRIMITIVE_TYPE_BYTE = 2,
                PRIMITIVE_TYPE_SHORT = 3,
                PRIMITIVE_TYPE_INTEGER = 4,
                PRIMITIVE_TYPE_LONG = 5,
                PRIMITIVE_TYPE_FLOAT = 6,
                PRIMITIVE_TYPE_DOUBLE = 7,
                PRIMITIVE_TYPE_UTF = 8,
                PRIMITIVE_TYPE_NULL = 9
            };

        };
    }
}


#endif //HAZELCAST_IOUtil

