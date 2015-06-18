#!/bin/bash

cd ../../hazelcast
git pull
mvn package -DskipTests
cp hazelcast/target/hazelcast-3.6-SNAPSHOT.jar ../cpp-client/java/
cd ../cpp-client/java
