//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "Credentials.h"


namespace hazelcast {
    namespace client {
        namespace protocol {
            Credentials::Credentials(std::string principal, std::string password)
            : principal(principal) {
                char const *pasw = password.c_str();
                std::vector<byte> pwd(pasw, pasw + 8); //TODO
                this->password = pwd;
            }


        }
    }
}
