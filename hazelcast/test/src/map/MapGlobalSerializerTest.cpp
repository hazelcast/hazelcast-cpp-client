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

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class MapGlobalSerializerTest : public ClientTestSupport {
            public:
                class UnknownObject {};

                class WriteReadIntGlobalSerializer : public serialization::StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 123;
                    }

                    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        return new int(5);
                    }
                };
            protected:

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig;
                    clientConfig.getSerializationConfig().setGlobalSerializer(
                            boost::shared_ptr<serialization::StreamSerializer>(new WriteReadIntGlobalSerializer()));
                    client = new HazelcastClient(clientConfig);
                    imap = new mixedtype::IMap(client->toMixedType().getMap("UnknownObject"));
                }

                static void TearDownTestCase() {
                    delete imap;
                    delete client;
                    delete instance;

                    imap = NULL;
                    client = NULL;
                    instance = NULL;
                }
                
                static HazelcastServer *instance;
                static HazelcastClient *client;
                static mixedtype::IMap *imap;
            };

            HazelcastServer *MapGlobalSerializerTest::instance = NULL;
            HazelcastClient *MapGlobalSerializerTest::client = NULL;
            mixedtype::IMap *MapGlobalSerializerTest::imap = NULL;
            
            TEST_F(MapGlobalSerializerTest, testPutUnknownObjectButGetIntegerValue) {
                MapGlobalSerializerTest::UnknownObject myObject;
                imap->put<int, MapGlobalSerializerTest::UnknownObject>(2, myObject);

                TypedData data = imap->get<int>(2);
                ASSERT_EQ(123, data.getType().typeId);
                std::auto_ptr<int> value = data.get<int>();
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);
            }

        }
    }
}
