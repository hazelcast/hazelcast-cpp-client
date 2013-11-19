//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "hazelcast/client/serialization/PortableContext.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include <zlib.h>
#include <algorithm>

namespace hazelcast {
    namespace client {
		typedef unsigned char byte;
        namespace serialization {


            PortableContext::PortableContext() {

            };

            PortableContext::~PortableContext() {
                std::vector<ClassDefinition *> values = versionedDefinitions.values();
                for (int i = 0; i < values.size(); i++) {
//                    delete values[i]; //TODO commented because of tests
                }

            };

            PortableContext::PortableContext(SerializationContext *serializationContext)
            : serializationContext(serializationContext) {

            };

            bool PortableContext::isClassDefinitionExists(int classId, int version) const {
                long long key = combineToLong(classId, version);
                return (versionedDefinitions.containsKey(key));
            };

            ClassDefinition *PortableContext::lookup(int classId, int version) {
                long long key = combineToLong(classId, version);
                return versionedDefinitions.get(key);

            };

            ClassDefinition *PortableContext::createClassDefinition(std::auto_ptr< std::vector<byte> > compressedBinary) {
                if (compressedBinary.get() == NULL || compressedBinary.get()->size() == 0) {
                    throw exception::IOException("PortableContext::createClassDefinition", "Illegal class-definition binary! ");
                }
                std::vector<byte> decompressed = decompress(*(compressedBinary.get()));

                DataInput dataInput(decompressed);
                ClassDefinition *cd = new ClassDefinition();
                cd->readData(dataInput);
                cd->setBinary(compressedBinary);

                long long key = combineToLong(cd->getClassId(), serializationContext->getVersion());

                serializationContext->registerNestedDefinitions(cd);
                ClassDefinition *currentCD = versionedDefinitions.putIfAbsent(key, cd);
                if (currentCD == NULL) {
                    return cd;
                } else {
                    return currentCD;
                }
            };

            ClassDefinition *PortableContext::registerClassDefinition(ClassDefinition *cd) {
                if (cd->getVersion() < 0) {
                    cd->setVersion(serializationContext->getVersion());
                }

                if (cd->getBinary().size() == 0) {
                    DataOutput output;
                    cd->writeData(output);
                    std::auto_ptr< std::vector<byte> > binary = output.toByteArray();
                    compress(*(binary.get()));
                    cd->setBinary(binary);
                }

                long long key = combineToLong(cd->getClassId(), cd->getVersion());
                serializationContext->registerNestedDefinitions(cd);
                ClassDefinition *currentCD = versionedDefinitions.putIfAbsent(key, cd);
                if (currentCD == NULL) {
                    return cd;
                } else {
                    return currentCD;
                }
            };

            void PortableContext::compress(std::vector<byte> &binary) {
                uLong ucompSize = binary.size();
                uLong compSize = compressBound(ucompSize);
				std::vector<byte> uncompressedTemp(binary.size());
                for (int i = 0; i < binary.size(); i++)
                    uncompressedTemp[i] = binary[i];

				byte* compressedTemp = new byte[compSize];

                int err = compress2((Bytef *) &(compressedTemp[0]), &compSize, (Bytef *) &(uncompressedTemp[0]), ucompSize, Z_BEST_COMPRESSION);
                switch (err) {
                    case Z_BUF_ERROR:
                        throw exception::IOException("PortableContext::compress", "not enough room in the output buffer at compression");
                    case Z_DATA_ERROR:
                        throw exception::IOException("PortableContext::compress", "data is corrupted at compression");
                    case Z_MEM_ERROR:
                        throw exception::IOException("PortableContext::compress", "there was not  enough memory at compression");
                }
				binary.resize(compSize);
				std::copy(compressedTemp, compressedTemp + compSize , binary.begin());
				delete compressedTemp;

            };

            std::vector<byte> PortableContext::decompress(std::vector<byte> const &binary) const {
                uLong compSize = binary.size();

                uLong ucompSize = 512;
                byte *temp = NULL;
				std::vector<byte> compressedTemp(binary.size());
                for (int i = 0; i < binary.size(); i++)
                    compressedTemp[i] = binary[i];
                int err = Z_OK;
                do {
                    ucompSize *= 2;
                    delete [] temp;
                    temp = new byte[ucompSize];
                    err = uncompress((Bytef *) temp, &ucompSize, (Bytef *) &(compressedTemp[0]), compSize);
                    switch (err) {
                        case Z_BUF_ERROR:
                            throw exception::IOException("PortableContext::compress", "not enough room in the output buffer at decompression");
                        case Z_DATA_ERROR:
                            throw exception::IOException("PortableContext::compress", "data is corrupted at decompression");
                        case Z_MEM_ERROR:
                            throw exception::IOException("PortableContext::compress", "there was not  enough memory at decompression");
                    }
                } while (err != Z_OK);
                std::vector<byte> decompressed(temp, temp + ucompSize);
                delete [] temp;
                return decompressed;
            };

            long long PortableContext::combineToLong(int x, int y) const {
				return ((long long)x) << 32 |  ((long long) y) & 0xFFFFFFFL;
			};

            int PortableContext::extractInt(long long value, bool lowerBits) const {
                return (lowerBits) ? (int) value : (int) (value >> 32);
            };

        }
    }
}