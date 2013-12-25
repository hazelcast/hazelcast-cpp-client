//
// Created by sancar koyunlu on 23/12/13.
//

#include "DataAdapter.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            DataAdapter::DataAdapter()
            :status(stType)
            , factoryId(0)
            , classId(0)
            , version(0)
            , classDefSize(0)
            , skipClassDef(false)
            , bytesRead(0) {

            }

            Data &DataAdapter::getData() {
                assert(!isStatusSet(stAll));
                return *data;
            }


            bool DataAdapter::readFrom(util::CircularBuffer &source) {
                if (data.get() == NULL) {
                    data.reset(new Data());
                }
                if (!isStatusSet(stType)) {
                    if (source.remaining() < 4) {
                        return false;
                    }
                    data->type = source.readInt();
                    setStatus(stType);
                }
                if (!isStatusSet(stClassId)) {
                    if (source.remaining() < 4) {
                        return false;
                    }
                    classId = source.readInt();
                    setStatus(stClassId);
                    if (classId == Data::NO_CLASS_ID) {
                        setStatus(stFactoryId);
                        setStatus(stVersion);
                        setStatus(stClassDefSize);
                        setStatus(stClassDef);
                    }
                }
                if (!isStatusSet(stFactoryId)) {
                    if (source.remaining() < 4) {
                        return false;
                    }
                    factoryId = source.readInt();
                    data->isError = (factoryId == protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY)
                            && (classId == protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID);
                    setStatus(stFactoryId);
                }
                if (!isStatusSet(stVersion)) {
                    if (source.remaining() < 4) {
                        return false;
                    }
                    version = source.readInt();
                    setStatus(stVersion);
                }
                if (!isStatusSet(stClassDef)) {
                    if (!skipClassDef) {
                        if (context->isClassDefinitionExists(factoryId, classId, version)) {
                            data->cd = context->lookup(factoryId, classId, version);
                            skipClassDef = true;
                        }
                    }
                    if (!isStatusSet(stClassDefSize)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        classDefSize = source.readInt();
                        setStatus(stClassDefSize);
                    }
                    if (!isStatusSet(stClassDef)) {
                        if (source.remaining() < classDefSize) {
                            return false;
                        }
                        if (skipClassDef) {
                            source.advance(classDefSize);
                        } else {
                            std::auto_ptr< std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                            source.readFully(*(classDefBytes.get()));
                            data->cd = context->createClassDefinition(factoryId, classDefBytes);
                        }
                        setStatus(stClassDef);
                    }
                }
                if (!isStatusSet(stSize)) {
                    if (source.remaining() < 4) {
                        return false;
                    }
                    int valueSize = source.readInt();
                    data->buffer.reset(new std::vector<byte>(valueSize));
                    setStatus(stSize);
                }
                if (!isStatusSet(stValue)) {
                    bytesRead += source.read(&((*(data->buffer))[0]), data->buffer->size() - bytesRead);
                    if (bytesRead != data->buffer->size()) {
                        return false;
                    }
                    setStatus(stValue);
                }

                if (!isStatusSet(stHash)) {
                    if (source.remaining() < 4) {
                        return false;
                    }
                    data->partitionHash = source.readInt();
                    setStatus(stHash);
                }
                setStatus(stAll);
                return true;
            }

            void DataAdapter::setStatus(StatusBit bit) {
                status |= 1 << (int) bit;
            }

            bool DataAdapter::isStatusSet(StatusBit bit) {
                return (status & 1 << (int) bit) != 0;
            }
        }
    }
}
