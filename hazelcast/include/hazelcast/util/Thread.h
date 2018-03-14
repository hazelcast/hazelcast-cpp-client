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
#ifndef HAZELCAST_UTIL_THREAD_H_
#define HAZELCAST_UTIL_THREAD_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include "hazelcast/util/WindowsThread.inl"
#else
#include "hazelcast/util/PosixThread.inl"
#endif

#endif //HAZELCAST_UTIL_THREAD_H_


