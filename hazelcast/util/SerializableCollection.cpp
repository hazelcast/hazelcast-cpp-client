//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "SerializableCollection.h"

namespace hazelcast {
    namespace util {
        SerializableCollection::SerializableCollection() {

        };
        
        SerializableCollection::~SerializableCollection(){
            vector<hazelcast::client::serialization::Data *>::iterator it;
            for (it = datas.begin(); it != datas.end(); ++it) {
                delete (*it);
            }
        };

        vector<hazelcast::client::serialization::Data *>  SerializableCollection::getCollection() const {
            return datas;
        };
    }
}