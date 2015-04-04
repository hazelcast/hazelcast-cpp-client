//
// Created by İhsan Demir on 25/03/15.
//


#ifndef HAZELCAST_PortableReaderBase_H_
#define HAZELCAST_PortableReaderBase_H_

#include "hazelcast/client/serialization/ObjectDataInput.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ClassDefinition;

            namespace pimpl {

                class DataInput;

				class HAZELCAST_API PortableReaderBase {

                public:
                    PortableReaderBase(PortableContext &portableContext,
                            DataInput &input,
                            boost::shared_ptr<ClassDefinition> cd);

                    virtual ~PortableReaderBase();

                    virtual int readInt(const char *fieldName, bool skipTypeCheck = false);

                    virtual long readLong(const char *fieldName, bool skipTypeCheck = false);

                    virtual bool readBoolean(const char *fieldName, bool skipTypeCheck = false);

                    virtual byte readByte(const char *fieldName, bool skipTypeCheck = false);

                    virtual char readChar(const char *fieldName, bool skipTypeCheck = false);

                    virtual double readDouble(const char *fieldName, bool skipTypeCheck = false);

                    virtual float readFloat(const char *fieldName, bool skipTypeCheck = false);

                    virtual short readShort(const char *fieldName, bool skipTypeCheck = false);

                    virtual std::string readUTF(const char *fieldName);

                    virtual std::vector<byte> readByteArray(const char *fieldName);

                    virtual std::vector<char> readCharArray(const char *fieldName);

                    virtual std::vector<int> readIntArray(const char *fieldName);

                    virtual std::vector<long> readLongArray(const char *fieldName);

                    virtual std::vector<double> readDoubleArray(const char *fieldName);

                    virtual std::vector<float> readFloatArray(const char *fieldName);

                    virtual std::vector<short> readShortArray(const char *fieldName);

                    ObjectDataInput &getRawDataInput();

                    void end();

                protected:
                    void getPortableInstance(char const *fieldName,
                            Portable * &portableInstance);

                    void getPortableInstancesArray(char const *fieldName,
                            std::vector<Portable *> &portableInstances);

                    void setPosition(char const * , FieldType const& fieldType, bool skipTypeCheck = false);

                    boost::shared_ptr<ClassDefinition> cd;
                    DataInput &dataInput;
                private:
                    const SerializerHolder &serializerHolder;
                    int finalPosition;
                    ObjectDataInput objectDataInput;
                    int offset;
                    bool raw;

                    void checkFactoryAndClass(FieldDefinition fd, int factoryId, int classId) const;

                    void read(DataInput &dataInput, Portable &object, int factoryId, int classId) const;

                    int readPosition(const char *, FieldType const& fieldType, bool skipTypeCheck = false);
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PortableReaderBase_H_
