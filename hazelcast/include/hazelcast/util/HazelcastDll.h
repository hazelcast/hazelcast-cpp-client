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
#ifndef HAZELCAST_HAZELCASTDLL
#define HAZELCAST_HAZELCASTDLL

#if defined(__SIZEOF_INT128__)
#  define BOOST_HAS_INT128
#endif

//# pragma warning(push)
//#pragma warning(disable: 4251) 	
//# pragma warning(pop)

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    #if !(defined(HAZELCAST_USE_STATIC) || defined(HAZELCAST_USE_SHARED))
        #ifndef HAZELCAST_EXPORTS
            #error Please define compilation flag HAZELCAST_USE_STATIC or HAZELCAST_USE_SHARED.
        #endif
    #endif

    #if defined(HAZELCAST_USE_STATIC)
        #define HAZELCAST_API
    #elif defined(HAZELCAST_USE_SHARED)
        #define HAZELCAST_API __declspec(dllimport)
    #else //HAZELCAST_EXPORTS case
        #if HAZELCAST_EXPORTS == 1
            #define HAZELCAST_API __declspec(dllexport)
        #else
            #define HAZELCAST_API
        #endif
    #endif

#else
    #define HAZELCAST_API
 #endif
namespace hazelcast{
    typedef unsigned char byte;
}

#endif // HAZELCAST_HAZELCASTDLL
