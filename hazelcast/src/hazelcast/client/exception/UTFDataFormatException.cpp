//
// Created by ihsan demir on 15/10/15.
// Copyright (c) 2015 hazelcast. All rights reserved.


#include "hazelcast/client/exception/UTFDataFormatException.h"


namespace hazelcast {
    namespace client {
        namespace exception {
            UTFDataFormatException::UTFDataFormatException(const std::string &source, const std::string &message)
            : IException(source, message) {

            }

            UTFDataFormatException::~UTFDataFormatException() throw(){

            }

            char const *UTFDataFormatException::what() const throw() {
                return IException::what();
            }
        }

    }
}

