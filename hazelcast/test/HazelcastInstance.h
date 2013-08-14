//
//  hazelcastInstance.h
//  hazelcast
//
//  Created by Batikan Turkmen on 14.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef __hazelcast__hazelcastInstance__
#define __hazelcast__hazelcastInstance__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

class HazelcastInstance {
public:
    HazelcastInstance();
    
    void terminate();

    ~HazelcastInstance();
private:
    std::string pid;
    bool isTerminated;
};

#endif /* defined(__hazelcast__hazelcastInstance__) */
