//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION
#define HAZELCAST_CLASS_DEFINITION


#include "hazelcast/client/serialization/pimpl/FieldDefinition.h"
#include <map>
#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class DataInput;

                class DataOutput;

                class HAZELCAST_API ClassDefinition {
                public:

                    ClassDefinition();

                    ClassDefinition(int factoryId, int classId, int version);

                    void add(FieldDefinition &);

                    void add(boost::shared_ptr<ClassDefinition>);

                    const FieldDefinition &get(const char *);

                    std::vector<boost::shared_ptr<ClassDefinition>  > &getNestedClassDefinitions();

                    bool hasField(const char *fieldName) const;

                    FieldType getFieldType(const char *fieldName) const;

                    int getFieldCount() const;

                    int getFactoryId() const;

                    int getClassId() const;

                    int getVersion() const;

                    const std::vector<byte> &getBinary() const;

                    void setBinary(std::auto_ptr < std::vector<byte> >);

                    void setVersion(int);

                    void writeData(DataOutput &dataOutput);

                    void readData(DataInput &dataInput);

                private:
                    int factoryId;
                    int classId;
                    int version;

                    ClassDefinition(const ClassDefinition &);

                    ClassDefinition &operator = (const ClassDefinition &rhs);

                    std::vector<FieldDefinition> fieldDefinitions;
                    std::map<std::string, FieldDefinition> fieldDefinitionsMap;
                    std::vector<boost::shared_ptr<ClassDefinition>  > nestedClassDefinitions;

                    std::auto_ptr< std::vector<byte> > binary;

                };
            }
        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION */

