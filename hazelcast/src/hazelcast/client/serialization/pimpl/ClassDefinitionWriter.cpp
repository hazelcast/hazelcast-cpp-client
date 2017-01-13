/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                ClassDefinitionWriter::ClassDefinitionWriter(PortableContext& portableContext, ClassDefinitionBuilder& builder)
                : builder(builder)
                , context(portableContext) {
                }

                boost::shared_ptr<ClassDefinition> ClassDefinitionWriter::registerAndGet() {
                    boost::shared_ptr<ClassDefinition> cd = builder.build();
                    return context.registerClassDefinition(cd);
                }

                void ClassDefinitionWriter::writeInt(const char *fieldName, int value) {
                    builder.addIntField(fieldName);
                }

                void ClassDefinitionWriter::writeLong(const char *fieldName, long value) {
                    builder.addLongField(fieldName);
                }

                void ClassDefinitionWriter::writeBoolean(const char *fieldName, bool value) {

                    builder.addBooleanField(fieldName);
                }

                void ClassDefinitionWriter::writeByte(const char *fieldName, byte value) {
                    builder.addByteField(fieldName);
                }

                void ClassDefinitionWriter::writeChar(const char *fieldName, int value) {
                    builder.addCharField(fieldName);
                }

                void ClassDefinitionWriter::writeDouble(const char *fieldName, double value) {

                    builder.addDoubleField(fieldName);
                }

                void ClassDefinitionWriter::writeFloat(const char *fieldName, float value) {
                    builder.addFloatField(fieldName);
                }

                void ClassDefinitionWriter::writeShort(const char *fieldName, short value) {
                    builder.addShortField(fieldName);
                }

                void ClassDefinitionWriter::writeUTF(const char *fieldName, const std::string *value) {

                    builder.addUTFField(fieldName);
                }

                void ClassDefinitionWriter::writeByteArray(const char *fieldName, const std::vector<byte> *values) {
                    builder.addByteArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeCharArray(const char *fieldName, const std::vector<char> *values) {
                    builder.addCharArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeIntArray(const char *fieldName, const std::vector<int> *values) {
                    builder.addIntArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeLongArray(const char *fieldName, const std::vector<long> *values) {
                    builder.addLongArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *values) {
                    builder.addDoubleArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeFloatArray(const char *fieldName, const std::vector<float> *values) {
                    builder.addFloatArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeShortArray(const char *fieldName, const std::vector<short> *values) {
                    builder.addShortArrayField(fieldName);
                }

                ObjectDataOutput& ClassDefinitionWriter::getRawDataOutput() {
                    return emptyDataOutput;
                }

                void ClassDefinitionWriter::end() {

                }

                boost::shared_ptr<ClassDefinition> ClassDefinitionWriter::createNestedClassDef(const Portable& portable) {
                    int version = pimpl::PortableVersionHelper::getVersion(&portable, context.getVersion());
                    ClassDefinitionBuilder definitionBuilder(portable.getFactoryId(), portable.getClassId(), version);

                    ClassDefinitionWriter nestedWriter(context, definitionBuilder);
                    PortableWriter portableWriter(&nestedWriter);
                    portable.writePortable(portableWriter);
                    return context.registerClassDefinition(definitionBuilder.build());
                }
            }
        }
    }
}
