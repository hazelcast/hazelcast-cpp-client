//
//  TestNamedPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestNamedPortable_h
#define Server_TestNamedPortable_h

#include <string>

using namespace hazelcast::client::serialization;

class TestNamedPortable {
public:
    TestNamedPortable() {
    };

    TestNamedPortable(std::string name, int k):name(name), k(k) {
    };

    virtual ~TestNamedPortable() {

    };

    virtual bool operator ==(TestNamedPortable& m) {
        if (this == &m)
            return true;
        if (k != m.k)
            return false;
        if (name.compare(m.name))
            return false;
        return true;
    };

    virtual bool operator !=(TestNamedPortable& m) {
        return !(*this == m);
    };
    std::string name;
    int k;
};


namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getTypeSerializerId(const TestNamedPortable& x) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            inline int getFactoryId(const TestNamedPortable& t) {
                return 1;
            }

            inline int getClassId(const TestNamedPortable& t) {
                return 3;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestNamedPortable& data) {
                writer["name"] << data.name;
                writer["myint"] << data.k;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestNamedPortable& data) {
                reader["name"] >> data.name;
                reader["myint"] >> data.k;
            };

        }
    }
}
#endif
