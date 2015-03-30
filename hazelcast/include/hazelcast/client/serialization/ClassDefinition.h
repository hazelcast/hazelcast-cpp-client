//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION
#define HAZELCAST_CLASS_DEFINITION


#include "hazelcast/client/serialization/FieldDefinition.h"
#include "hazelcast/util/ByteBuffer.h"
#include <map>
#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class DataInput;

                class DataOutput;
            }

            /**
            * ClassDefinition defines a class schema for Portable classes. It allows to query field names, types, class id etc.
            * It can be created manually using {@link ClassDefinitionBuilder}
            * or on demand during serialization phase.
            *
            * @see Portable
            * @see ClassDefinitionBuilder
            */
            class HAZELCAST_API ClassDefinition {
            public:

                /**
                * Constructor
                */
                ClassDefinition();

                /**
                * Constructor
                * @param factoryId factory id of class
                * @param classId id of class
                * @param version version of portable class
                */
                ClassDefinition(int factoryId, int classId, int version);

                /**
                * Internal API
                * @param fieldDefinition to be added
                */
                void addFieldDef(FieldDefinition& fieldDefinition);

                /**
                * Internal API
                * @param classDefinition to be added
                */
                void addClassDef(boost::shared_ptr<ClassDefinition> classDefinition);

                /**
                * @param fieldName field name
                * @return true if this class definition contains a field named by given name
                */
                bool hasField(char const *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return field definition by given name
                * @throws IllegalArgumentException when field not found
                */
                const FieldDefinition& getField(const char *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return field definition pointer by given name or null if the field does not exist
                */
                const FieldDefinition* getFieldIfExist(const char *fieldName) const;

                /**
                * @param fieldIndex index of the field
                * @return field definition by given index
                * @throws IllegalArgumentException when field not found
                */
                const FieldDefinition& getField(int fieldIndex) const;

                /**
                * @return all field names contained in this class definition
                */
                std::vector<std::string> getFieldNames() const;

                /**
                * @param fieldName name of the field
                * @return type of given field
                * @throws IllegalArgumentException
                */
                FieldType getFieldType(const char *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return type of given field type pointer or null if field does not exist.
                */
                const FieldType *getFieldTypeIfExists(const char *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return class id of given field
                * @throws IllegalArgumentException
                */
                int getFieldClassId(const char *fieldName) const;

                /**
                * @return total field count
                */
                int getFieldCount() const;

                /**
                * Internal API.
                * @return nested class definition vector
                */
                std::vector<boost::shared_ptr<ClassDefinition> >& getNestedClassDefinitions();

                /**
                * @return factory id
                */
                int getFactoryId() const;

                /**
                * @return class id
                */
                int getClassId() const;

                /**
                * @return version
                */
                int getVersion() const;

                /**
                * Internal API.
                * Returns byte array of class definition
                */
                const std::vector<byte>& getBinary() const;

                /**
                *  Internal API.
                *  Set internal byte compressed byte array
                *  @param binary compressed binary
                */
                void setBinary(hazelcast::util::ByteVector_ptr compressedBinary);

                /**
                * Internal API
                * @param version portable version
                */
                void setVersionIfNotSet(int version);

                /**
                * Internal API
                * @param dataOutput
                */
                void writeData(pimpl::DataOutput& dataOutput);

                /**
                * Internal API
                * @param dataInput
                */
                void readData(pimpl::DataInput& dataInput);

            private:
                int factoryId;
                int classId;
                int version;

                ClassDefinition(const ClassDefinition&);

                ClassDefinition& operator=(const ClassDefinition& rhs);

                std::vector<FieldDefinition> fieldDefinitions;
                std::map<std::string, FieldDefinition> fieldDefinitionsMap;
                std::vector<boost::shared_ptr<ClassDefinition> > nestedClassDefinitions;

                hazelcast::util::ByteVector_ptr binary;

            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 


#endif /* HAZELCAST_CLASS_DEFINITION */

