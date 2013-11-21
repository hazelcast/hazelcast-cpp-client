//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/exception/IOException.h"


namespace hazelcast {
    namespace client {
        namespace exception {
            IOException::IOException(const std::string &source, const std::string &message)
            : IException(source, message) {

            }

            IOException::~IOException() throw(){

            }

            char const *IOException::what() const throw() {
                return IException::what();
            }
        }

    }
}
