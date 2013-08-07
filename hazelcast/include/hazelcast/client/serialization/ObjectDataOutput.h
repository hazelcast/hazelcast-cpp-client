//
//  BufferedDataOutput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_OUTPUT
#define HAZELCAST_DATA_OUTPUT

#include "SerializationConstraints.h"
#include "ClassDefinitionWriter.h"
#include "PortableWriter.h"
#include <memory>
#include <vector>
#include <iosfwd>

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class ObjectDataOutput {
            public:

                ObjectDataOutput(SerializerHolder&, SerializationContext&);

                virtual ~ObjectDataOutput();

                std::auto_ptr< std::vector<byte> > toByteArray();

                void write(const std::vector<byte>& bytes);

                void writeBoolean(bool b);

                void writeByte(int i);

                void writeShort(int i);

                void writeChar(int i);

                void writeInt(int i);

                void writeLong(long l);

                void writeFloat(float v);

                void writeDouble(double v);

                void writeUTF(const std::string& s);

                void writeByteArray(const std::vector<byte>&  data);

                void writeCharArray(const std::vector<char>& bytes);

                void writeShortArray(const std::vector<short >&  data);

                void writeIntArray(const std::vector<int>&  data);

                void writeLongArray(const std::vector<long >&  data);

                void writeFloatArray(const std::vector<float >&  data);

                void writeDoubleArray(const std::vector<double >&  data);

                void writeByte(int index, int i);

                void writeInt(int index, int v);

                void writeNullObject();

                template<typename T>
                void writeObject(const Portable *portable) {
                    Is_Portable<T>();
                    const T *p = dynamic_cast<const T *>(portable);
                    writeBoolean(true);
                    writeInt(getSerializerId(*p));
                    boost::shared_ptr<ClassDefinition> cd = context.lookup(p->getFactoryId(), p->getClassId());
                    if (cd == NULL) {
                        ClassDefinitionWriter classDefinitionWriter(context.getVersion(), p->getFactoryId(), p->getClassId(), this);
                        boost::shared_ptr<ClassDefinition> cd = classDefinitionWriter.getOrBuildClassDefinition(*p);
                        cd->writeData(*this);
                    }
                    serializerHolder.getPortableSerializer().write(*this, *p);

                };

                template<typename T>
                void writeObject(const DataSerializable *dataSerializable) {
                    Is_DataSerializable<T>();
                    const T *p = dynamic_cast<const T *>(dataSerializable);
                    writeBoolean(true);
                    writeInt(getSerializerId(*p));
                    serializerHolder.getDataSerializer().write(*this, *p);
                };

                template<typename T>
                void writeObject(const void *serializable) {
                    const T *object = static_cast<const T *>(serializable);
                    int type = getSerializerId(*object);
                    writeBoolean(true);
                    writeInt(type);
                    SerializerBase *serializer = serializerHolder.serializerFor(type);
                    if (serializer) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);
                        s->write(*this, *object);
                    } else {
                        throw exception::IOException("ObjectDataOutput::writeObject", "No serializer found for serializerId :" + util::to_string(type) + ", typename :" + typeid(T).name());
                    }
                };

                template <typename T>
                void writeDataSerializable(T& object) {
                    writeBoolean(true);
                    writeInt(object.getFactoryId());
                    writeInt(object.getClassId());
                    object.writeData(*this);
                };

                template <typename T>
                boost::shared_ptr<ClassDefinition> getClassDefinition(const T& p) {
                    boost::shared_ptr<ClassDefinition> cd;

                    int factoryId = p.getFactoryId();
                    int classId = p.getClassId();
                    if (context.isClassDefinitionExists(factoryId, classId)) {
                        cd = context.lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context.getVersion(), context);
                        p.writePortable(classDefinitionWriter);
                        cd = classDefinitionWriter.getClassDefinition();
                        cd = context.registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(const T& p) {
                    boost::shared_ptr<ClassDefinition> cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, *this);
                    p.writePortable(portableWriter);
                    portableWriter.end();
                };


                int position();

                void position(int newPos);

                void reset();

                static int const STRING_CHUNK_SIZE = 16 * 1024;
                static int const DEFAULT_SIZE = 4 * 1024;

            private:
                std::auto_ptr< std::vector<byte> > outputStream;
                SerializerHolder& serializerHolder;
                SerializationContext& context;

                void writeShortUTF(const std::string&);

                ObjectDataOutput(const ObjectDataOutput& rhs);

                ObjectDataOutput& operator = (const ObjectDataOutput& rhs);

            };
        }
    }
}
#endif /* HAZELCAST_DATA_OUTPUT */
