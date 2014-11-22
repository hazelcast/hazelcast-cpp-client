//
// Created by sancar koyunlu on 18/11/14.
//

#include "hazelcast/client/serialization/pimpl/ClassDefinitionAdapter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/ByteBuffer.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                const int ClassDefinitionAdapter::CLASS_DEF_HEADER_SIZE = 16;
                const int ClassDefinitionAdapter::ST_PREPARED = 1;
                const int ClassDefinitionAdapter::ST_HEADER = 2;
                const int ClassDefinitionAdapter::ST_DATA = 3;
                const int ClassDefinitionAdapter::ST_SKIP_DATA = 4;

                ClassDefinitionAdapter::ClassDefinitionAdapter(Data& data, PortableContext& portableContext)
                : data(data)
                , context(portableContext)
                , status(0)
                , classDefCount(0)
                , classDefIndex(0)
                , bytesWrittenPerClassDefinition(0)
                , classDefSize(0)
                , factoryId(0)
                , bytesReadPerClassDefinition(0) {

                }


                bool ClassDefinitionAdapter::write(hazelcast::util::ByteBuffer& destination) {
                    if (!isStatusSet(ST_PREPARED)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }

                        classDefCount = data.getClassDefinitionCount();
                        classDefinitions = data.getClassDefinitions(context);

                        destination.writeInt(classDefCount);
                        setStatus(ST_PREPARED);
                    }

                    if (!writeAll(destination)) {
                        return false;
                    }
                    return true;
                }

                bool  ClassDefinitionAdapter::writeAll(hazelcast::util::ByteBuffer& destination) {
                    for (; classDefIndex < classDefCount; classDefIndex++) {
                        boost::shared_ptr<ClassDefinition> cd = classDefinitions[classDefIndex];

                        if (!writeHeader(*cd, destination)) {
                            return false;
                        }

                        if (!writeData(*cd, destination)) {
                            return false;
                        }

                        clearStatus(ST_HEADER);
                        clearStatus(ST_DATA);
                    }
                    return true;
                }

                bool  ClassDefinitionAdapter::writeHeader(ClassDefinition& cd, hazelcast::util::ByteBuffer& destination) {
                    if (isStatusSet(ST_HEADER)) {
                        return true;
                    }

                    if (destination.remaining() < CLASS_DEF_HEADER_SIZE) {
                        return false;
                    }

                    destination.writeInt(cd.getFactoryId());
                    destination.writeInt(cd.getClassId());
                    destination.writeInt(cd.getVersion());

                    std::vector<byte> const& binary = cd.getBinary();
                    destination.writeInt(binary.size());

                    setStatus(ST_HEADER);
                    return true;
                }

                bool  ClassDefinitionAdapter::writeData(ClassDefinition& cd, hazelcast::util::ByteBuffer& destination) {
                    if (isStatusSet(ST_DATA)) {
                        return true;
                    }

                    std::vector<byte> const& binary = cd.getBinary();
                    if (!flushBuffer(binary, destination)) {
                        return false;
                    }

                    setStatus(ST_DATA);
                    bytesWrittenPerClassDefinition = 0;
                    return true;
                }

                bool ClassDefinitionAdapter::flushBuffer(std::vector<byte> const& binary, hazelcast::util::ByteBuffer& destination) {
                    bytesWrittenPerClassDefinition += destination.readFrom(binary, bytesWrittenPerClassDefinition);
                    if (bytesWrittenPerClassDefinition != binary.size()) {
                        return false;
                    }
                    return true;
                }

                bool ClassDefinitionAdapter::read(hazelcast::util::ByteBuffer& source) {
                    if (!isStatusSet(ST_PREPARED)) {
                        if (source.remaining() < 4) {
                            return false;
                        }

                        classDefCount = (size_t)source.readInt();
                        metadata.reset(new std::vector<byte>(classDefCount * Data::HEADER_ENTRY_LENGTH));

                        setStatus(ST_PREPARED);
                    }

                    if (!readAll(source)) {
                        return false;
                    }
                    return true;
                }

                bool ClassDefinitionAdapter::readAll(hazelcast::util::ByteBuffer& source) {
                    for (; classDefIndex < classDefCount; classDefIndex++) {

                        if (!readHeader(source)) {
                            return false;
                        }

                        if (!readData(source)) {
                            return false;
                        }

                        clearStatus(ST_HEADER);
                        clearStatus(ST_DATA);
                    }
                    data.setHeader(metadata);
                    return true;
                }

                bool ClassDefinitionAdapter::readHeader(hazelcast::util::ByteBuffer& source) {
                    if (isStatusSet(ST_HEADER)) {
                        return true;
                    }

                    if (source.remaining() < CLASS_DEF_HEADER_SIZE) {
                        return false;
                    }

                    factoryId = source.readInt();
                    int classId = source.readInt();
                    int version = source.readInt();
                    classDefSize = (size_t)source.readInt();

                    util::writeIntToPos(*metadata, classDefIndex * Data::HEADER_ENTRY_LENGTH + Data::HEADER_FACTORY_OFFSET, factoryId);
                    util::writeIntToPos(*metadata, classDefIndex * Data::HEADER_ENTRY_LENGTH + Data::HEADER_CLASS_OFFSET, classId);
                    util::writeIntToPos(*metadata, classDefIndex * Data::HEADER_ENTRY_LENGTH + Data::HEADER_VERSION_OFFSET, version);


                    if (!context.isClassDefinitionExists(factoryId, classId, version)) {
                        clearStatus(ST_SKIP_DATA);
                    } else {
                        setStatus(ST_SKIP_DATA);
                    }
                    setStatus(ST_HEADER);
                    return true;
                }

                bool ClassDefinitionAdapter::readData(hazelcast::util::ByteBuffer& source) {
                    if (isStatusSet(ST_DATA)) {
                        return true;
                    }

                    if (isStatusSet(ST_SKIP_DATA)) {
                        using namespace std;
                        int skip = min(classDefSize, source.remaining());
                        source.skip(skip);

                        classDefSize -= skip;
                        if (classDefSize > 0) {
                            return false;
                        }
                        clearStatus(ST_SKIP_DATA);
                    } else {

                        buffer.reset(new std::vector<byte>(classDefSize));

                        bytesReadPerClassDefinition += source.writeTo((*(buffer)), bytesReadPerClassDefinition);
                        if (bytesReadPerClassDefinition != buffer->size()) {
                            return false;
                        }

                        context.createClassDefinition(factoryId, buffer);
                    }

                    setStatus(ST_DATA);
                    return true;
                }

                void ClassDefinitionAdapter::setStatus(int bit) {
                    status = util::setBit(status, bit);
                }

                bool ClassDefinitionAdapter::isStatusSet(int bit) const {
                    return util::isBitSet(status, bit);
                }

                void ClassDefinitionAdapter::clearStatus(int bit) {
                    status = util::clearBit(status, bit);
                }
            }
        }
    }
}