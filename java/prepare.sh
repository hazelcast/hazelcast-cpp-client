cd ../../hazelcast
git pull
mvn package -DskipTests
cp hazelcast/target/hazelcast-3.3-SNAPSHOT.jar ../cpp-client/java/
cd ../cpp-client/java
