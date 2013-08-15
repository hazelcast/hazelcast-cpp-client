//
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Batikan Turkmen on 14.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#include "hazelcastInstance.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

HazelcastInstance::HazelcastInstance()
:isTerminated(false){
    system("sh ./hazel.sh");
    //system("sh /Users/turkmen/Library/Developer/Xcode/DerivedData/hazelcast-cfqufpgluqgyelgyifqiadetwfhs/Build/Products/Debug/hazel.sh");
    
    ifstream infile("instance.txt");
    
    if (!infile.is_open()) {
        cout << "There was a problem opening file for reading." << endl;
    }
    
    if (infile >> pid) {
        cout << "PID value from file is " << pid << endl;
    }
    sleep(3);
};

void HazelcastInstance::terminate(){
    string command = "kill ";
    command += pid;
    
    system(command.c_str());
    isTerminated = true;
    
};

HazelcastInstance::~HazelcastInstance(){
    if(!isTerminated)
        terminate();
};

