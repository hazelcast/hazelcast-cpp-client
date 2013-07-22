//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "hazelcast/client/serialization/PortableContext.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include <zlib.h>

namespace hazelcast {
    namespace client {
        namespace serialization {


            PortableContext::PortableContext() {

            };

            PortableContext::~PortableContext() {

            };

            PortableContext::PortableContext(SerializationContext *serializationContext)
            : serializationContext(serializationContext) {

            };

            bool PortableContext::isClassDefinitionExists(int classId, int version) const {
                long key = combineToLong(classId, version);
                return (versionedDefinitions.containsKey(key));
            };

            boost::shared_ptr<ClassDefinition> PortableContext::lookup(int classId, int version) {
                long key = combineToLong(classId, version);
                return versionedDefinitions.get(key);

            };

            boost::shared_ptr<ClassDefinition> PortableContext::createClassDefinition(std::auto_ptr< std::vector<byte> > binary) {

                std::vector<byte> decompressed = decompress(*(binary.get()));

                BufferedDataInput dataInput(decompressed);
                boost::shared_ptr<ClassDefinition> cd(new ClassDefinition);
                cd->readData(dataInput);
                cd->setBinary(binary);

                long key = combineToLong(cd->getClassId(), serializationContext->getVersion());

                if (versionedDefinitions.containsKey(key) == 0) {
                    serializationContext->registerNestedDefinitions(cd);
                    boost::shared_ptr<ClassDefinition> pDefinition = versionedDefinitions.putIfAbsent(key, cd);
                    return pDefinition == NULL ? cd : pDefinition;
                }
                return versionedDefinitions.get(key);
            };

            boost::shared_ptr<ClassDefinition> PortableContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                if (cd->getVersion() < 0) {
                    cd->setVersion(serializationContext->getVersion());
                }
                if (!isClassDefinitionExists(cd->getClassId(), cd->getVersion())) {
                    if (cd->getBinary().size() == 0) {
                        BufferedDataOutput output;
                        cd->writeData(output);
                        std::auto_ptr< std::vector<byte> > binary = output.toByteArray();
                        compress(*(binary.get()));
                        cd->setBinary(binary);
                    }
                    long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    boost::shared_ptr<ClassDefinition> pDefinition = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    return pDefinition == NULL ? cd : pDefinition;
                }
                return lookup(cd->getClassId(), cd->getVersion());
            };

            void PortableContext::compress(std::vector<byte>& binary) {
                uLong ucompSize = binary.size();
                uLong compSize = compressBound(ucompSize);
                byte uncompressedTemp[binary.size()];
                for (int i = 0; i < binary.size(); i++)
                    uncompressedTemp[i] = binary[i];

                byte compressedTemp[compSize];
                int err = compress2((Bytef *) compressedTemp, &compSize, (Bytef *) uncompressedTemp, ucompSize, Z_BEST_COMPRESSION);
                switch (err) {
                    case Z_BUF_ERROR:
                        throw exception::IOException("PortableContext::compress", "not enough room in the output buffer at compression");
                    case Z_DATA_ERROR:
                        throw exception::IOException("PortableContext::compress", "data is corrupted at compression");
                    case Z_MEM_ERROR:
                        throw exception::IOException("PortableContext::compress", "there was not  enough memory at compression");
                }
                std::vector<byte> compressed(compressedTemp, compressedTemp + compSize);
                binary = compressed;
            };

            std::vector<byte> PortableContext::decompress(std::vector<byte> const & binary) const {
                uLong compSize = binary.size();

                uLong ucompSize = 512;
                byte *temp = NULL;
                byte compressedTemp[binary.size()];
                for (int i = 0; i < binary.size(); i++)
                    compressedTemp[i] = binary[i];
                int err = Z_OK;
                do {
                    ucompSize *= 2;
                    delete [] temp;
                    temp = new byte[ucompSize];
                    err = uncompress((Bytef *) temp, &ucompSize, (Bytef *) compressedTemp, compSize);
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

            long PortableContext::combineToLong(int x, int y) const {
                return ((long) x << 32) | ((long) y & 0xFFFFFFFL);
            };

            int PortableContext::extractInt(long value, bool lowerBits) const {
                return (lowerBits) ? (int) value : (int) (value >> 32);
            };

        }
    }
}