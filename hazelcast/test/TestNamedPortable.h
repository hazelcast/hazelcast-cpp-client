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
#include "SerializationConstants.h"

using namespace hazelcast::client::serialization;

class TestNamedPortable {
public:
    TestNamedPortable() {
    };

    TestNamedPortable(std::string name, int k):name(name), k(k) {
    };

    inline int getTypeSerializerId() const {
        return SerializationConstants::CONSTANT_TYPE_PORTABLE;
    };

    inline int getFactoryId() const {
        return 1;
    }

    inline int getClassId() const {
        return 3;
    }

    template<typename HzWriter>
    inline void writePortable(HzWriter& writer) const {
        writer["name"] << name;
        writer["myint"] << k;
    };

    template<typename HzReader>
    inline void readPortable(HzReader& reader) {
        reader["name"] >> name;
        reader["myint"] >> k;
    };

    virtual ~TestNamedPortable() {

    };

    virtual bool operator ==(const TestNamedPortable& m) const {
        if (this == &m)
            return true;
        if (k != m.k)
            return false;
        if (name.compare(m.name))
            return false;
        return true;
    };

    virtual bool operator !=(const TestNamedPortable& m) const {
        return !(*this == m);
    };
    std::string name;
    int k;
};


#endif
