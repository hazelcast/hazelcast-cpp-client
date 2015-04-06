//
// Created by sancar koyunlu on 05/04/15.
//


#ifndef HAZELCAST_TestSerializationConstants
#define HAZELCAST_TestSerializationConstants


namespace hazelcast {
    namespace client {
        namespace test {
            namespace TestSerializationConstants {

                int const TEST_PORTABLE_FACTORY = 1;
                int const TEST_DATA_FACTORY = 1;

                int const CHILD_TEMPLATED_PORTABLE_1 = 1;
                int const CHILD_TEMPLATED_PORTABLE_2 = 2;
                int const OBJECT_CARRYING_PORTABLE = 4;
                int const PARENT_TEMPLATED_CONSTANTS = 5;
                int const TEST_DATA_SERIALIZABLE = 6;
                int const TEST_INNER_PORTABLE = 7;
                int const TEST_INVALID_READ_PORTABLE = 8;
                int const TEST_INVALID_WRITE_PORTABLE = 9;
                int const TEST_MAIN_PORTABLE = 10;
                int const TEST_NAMED_PORTABLE = 11;
                int const TEST_NAMED_PORTABLE_2 = 12;
                int const TEST_NAMED_PORTABLE_3 = 13;
                int const TEST_RAW_DATA_PORTABLE = 14;

                int const EMPLOYEE_FACTORY = 666;
                int const EMPLOYEE = 2;
            }
        }
    }
}

#endif //HAZELCAST_TestSerializationConstants
