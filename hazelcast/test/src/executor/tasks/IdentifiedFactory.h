/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_TEST_EXECUTOR_TASKS_IDENTIFIEDFACTORY_H
#define HAZELCAST_CLIENT_TEST_EXECUTOR_TASKS_IDENTIFIEDFACTORY_H

class IdentifiedFactory {
public:
    static const int FACTORY_ID = 66;

    enum CLASS_IDS {
        APPENDCALLABLE = 5,
        CANCELLATIONAWARETASK = 6,
        FAILINGCALLABLE = 7,
        GETMEMBERUUIDTASK = 8,
        MAPPUTPARTITIONAWARECALLABLE = 9,
        NULLCALLABLE = 12,
        SELECTALLMEMBERS = 13,
        SELECTNOMEMBERS = 14,
        SERIALIZEDCOUNTERCALLABLE = 15,
        TASKWITHUNSERIALIZABLERESPONSE = 16,
    };
};

#endif //HAZELCAST_CLIENT_TEST_EXECUTOR_TASKS_IDENTIFIEDFACTORY_H
