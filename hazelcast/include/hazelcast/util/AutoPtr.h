//
//  UniquePtr.h
//  cpp-client
//
//  Created by sancar koyunlu on 10/7/13.
//  Copyright (c) 2013 hazelcast. All rights reserved.
//


#ifndef hazelcast_cpp_client_UniquePtr_h
#define hazelcast_cpp_client_UniquePtr_h

#include "hazelcast/client/exception/IException.h"
#include <cstdlib>

namespace hazelcast {
    namespace util {
        template <typename T>
        class HAZELCAST_API AutoPtr {
        public:
            AutoPtr():ptr(NULL) {

            };

            AutoPtr(T *ptr):ptr(ptr) {

            };

            AutoPtr(AutoPtr &rhs):ptr(rhs.ptr) {
                rhs.ptr = NULL;
            };


            AutoPtr(const AutoPtr &rhs):ptr(rhs.ptr) {
                rhs.ptr = NULL;
            }

            void operator = (const AutoPtr &rhs) {
                this->ptr = rhs.ptr;
                rhs.ptr = NULL;
            }

            void operator = (AutoPtr &rhs) {
                this->ptr = rhs.ptr;
                rhs.ptr = NULL;

            };

            void operator = (T *ptr) {
                this->ptr = ptr;
            };

            bool isEmpty() const {
                return ptr == NULL;
            }

            ~AutoPtr() {
                delete ptr;
            }

            T& operator *() {
                if (ptr == NULL)
                    throw client::exception::IException("UniquePtr::operator*()", "Trying to access unitialized memory");
                return *ptr;
            };

            const T& operator *() const {
                if (ptr == NULL)
                    throw client::exception::IException("UniquePtr::operator*() const", "Trying to access unitialized memory");
                return *ptr;
            };

            T *operator ->() {
                return ptr;
            };

            const T *operator ->() const {
                return ptr;
            };

            T& get() const {
                if (ptr == NULL)
                    throw client::exception::IException("UniquePtr::get() const", "Trying to access unitialized memory");
                return *ptr;
            }

        private:
            mutable T *ptr;


        };
    }
}


#endif
