#!/bin/sh

java -cp /Users/turkmen/Desktop/hazelcast-3.0-RC1/lib/hazelcast-3.0-RC1.jar:.  creator&
echo $! > instance.txt
