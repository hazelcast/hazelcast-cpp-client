//
//  TestNamedPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestNamedPortable_h
#define Server_TestNamedPortable_h

#include "SerializationConstants.h"
#include "Portable.h"
#include <string>

using namespace hazelcast::client::serialization;

class TestNamedPortable : public hazelcast::client::Portable{
public:
    TestNamedPortable() {
    };

    TestNamedPortable(std::string name, int k):name(name), k(k) {
    };

    inline int getFactoryId() const {
        return 1;
    }

    inline int getClassId() const {
        return 3;
    }

    template<typename HzWriter>
    inline void writePortable(HzWriter& writer) const {
        writer.writeUTF("name", name);
        writer.writeInt("myint", k);
    };

    template<typename HzReader>
    inline void readPortable(HzReader& reader) {
        name = reader.readUTF("name");
        k = reader.readInt("myint");
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
