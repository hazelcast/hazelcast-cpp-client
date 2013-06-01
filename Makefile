-include $(OBJS:.o=.d)
CC=clang++
OUT=hazelcastClient.out
RM=rm -f

OBJS=./obj/hazelcast/util/Thread.o ./obj/hazelcast/client/protocol/Credentials.o ./obj/hazelcast/client/connection/ClusterListenerThread.o ./obj/hazelcast/client/serialization/ConstantSerializers.o ./obj/hazelcast/util/AtomicInteger.o ./obj/hazelcast/client/ISemaphore.o ./obj/hazelcast/client/HazelcastException.o ./obj/hazelcast/client/serialization/BufferedDataInput.o ./obj/hazelcast/client/serialization/PortableContext.o ./obj/hazelcast/client/serialization/PortableWriter.o ./obj/hazelcast/client/impl/AbstractLoadBalancer.o ./obj/hazelcast/client/serialization/SerializationService.o ./obj/hazelcast/client/serialization/PortableReader.o ./obj/hazelcast/client/IdGenerator.o ./obj/hazelcast/client/serialization/FieldDefinition.o ./obj/hazelcast/client/map/RemoveRequest.o ./obj/hazelcast/client/connection/HeartBeatChecker.o ./obj/hazelcast/client/spi/ClusterService.o ./obj/hazelcast/client/connection/Member.o ./obj/hazelcast/client/ClientConfig.o ./obj/hazelcast/client/serialization/Data.o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.o ./obj/hazelcast/client/ICountDownLatch.o ./obj/hazelcast/client/protocol/ClientPingRequest.o ./obj/hazelcast/client/serialization/ClassDefinitionWriter.o ./obj/hazelcast/client/HazelcastClient.o ./obj/hazelcast/client/protocol/SocketPool.o ./obj/hazelcast/client/connection/Connection.o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.o ./obj/hazelcast/client/serialization/NullPortable.o ./obj/hazelcast/client/protocol/AuthenticationRequest.o ./obj/hazelcast/client/protocol/HazelcastServerError.o ./obj/hazelcast/util/ConcurrentQueue.o ./obj/hazelcast/client/connection/Socket.o ./obj/hazelcast/client/serialization/BufferedDataOutput.o ./obj/hazelcast/client/serialization/InputSocketStream.o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.o ./obj/hazelcast/client/serialization/MorphingPortableReader.o ./obj/hazelcast/client/map/PutRequest.o ./obj/hazelcast/client/map/GetRequest.o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.o ./obj/hazelcast/client/IAtomicLong.o ./obj/hazelcast/util/Lock.o ./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o ./obj/hazelcast/client/Address.o ./obj/hazelcast/client/serialization/DataSerializer.o ./obj/hazelcast/util/Util.o ./obj/hazelcast/util/ConcurrentMap.o ./obj/hazelcast/client/Cluster.o ./obj/hazelcast/client/spi/ClientContext.o ./obj/hazelcast/client/connection/MembershipEvent.o ./obj/hazelcast/client/protocol/Principal.o ./obj/hazelcast/client/connection/ConnectionManager.o ./obj/hazelcast/client/serialization/OutputSocketStream.o ./obj/hazelcast/client/serialization/ClassDefinition.o ./obj/hazelcast/client/spi/InvocationService.o ./obj/main.o ./obj/hazelcast/client/connection/ConnectionPool.o ./obj/hazelcast/client/serialization/SerializationContext.o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.o ./obj/hazelcast/client/impl/RoundRobinLB.o ./obj/hazelcast/util/SerializableCollection.o ./obj/hazelcast/client/protocol/AddMembershipListenerRequest.o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.o ./obj/hazelcast/client/GroupConfig.o ./obj/hazelcast/client/serialization/PortableSerializer.o 

CFLAGS=
LFLAGS= /usr/local/lib/libz.a

all: $(OUT)
	echo "Finished."

$(OUT): $(OBJS)
	echo "Linking $(OUT)..."
	$(CC)  $(OBJS) -o $(OUT) $(LFLAGS)

./obj/hazelcast/util/Thread.o: hazelcast/util/Thread.cpp
	echo "Compiling hazelcast/util/Thread.cpp  to ./obj/hazelcast/util/Thread.o ..."
	$(CC) $(CFLAGS) -c hazelcast/util/Thread.cpp -o ./obj/hazelcast/util/Thread.o
	$(CC) $(CFLAGS) -MM hazelcast/util/Thread.cpp > hazelcast/util/Thread.d

./obj/hazelcast/client/protocol/Credentials.o: hazelcast/client/protocol/Credentials.cpp
	echo "Compiling hazelcast/client/protocol/Credentials.cpp  to ./obj/hazelcast/client/protocol/Credentials.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/protocol/Credentials.cpp -o ./obj/hazelcast/client/protocol/Credentials.o
	$(CC) $(CFLAGS) -MM hazelcast/client/protocol/Credentials.cpp > hazelcast/client/protocol/Credentials.d

./obj/hazelcast/client/connection/ClusterListenerThread.o: hazelcast/client/connection/ClusterListenerThread.cpp
	echo "Compiling hazelcast/client/connection/ClusterListenerThread.cpp  to ./obj/hazelcast/client/connection/ClusterListenerThread.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/ClusterListenerThread.cpp -o ./obj/hazelcast/client/connection/ClusterListenerThread.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/ClusterListenerThread.cpp > hazelcast/client/connection/ClusterListenerThread.d

./obj/hazelcast/client/serialization/ConstantSerializers.o: hazelcast/client/serialization/ConstantSerializers.cpp
	echo "Compiling hazelcast/client/serialization/ConstantSerializers.cpp  to ./obj/hazelcast/client/serialization/ConstantSerializers.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ConstantSerializers.cpp -o ./obj/hazelcast/client/serialization/ConstantSerializers.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ConstantSerializers.cpp > hazelcast/client/serialization/ConstantSerializers.d

./obj/hazelcast/util/AtomicInteger.o: hazelcast/util/AtomicInteger.cpp
	echo "Compiling hazelcast/util/AtomicInteger.cpp  to ./obj/hazelcast/util/AtomicInteger.o ..."
	$(CC) $(CFLAGS) -c hazelcast/util/AtomicInteger.cpp -o ./obj/hazelcast/util/AtomicInteger.o
	$(CC) $(CFLAGS) -MM hazelcast/util/AtomicInteger.cpp > hazelcast/util/AtomicInteger.d

./obj/hazelcast/client/ISemaphore.o: hazelcast/client/ISemaphore.cpp
	echo "Compiling hazelcast/client/ISemaphore.cpp  to ./obj/hazelcast/client/ISemaphore.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/ISemaphore.cpp -o ./obj/hazelcast/client/ISemaphore.o
	$(CC) $(CFLAGS) -MM hazelcast/client/ISemaphore.cpp > hazelcast/client/ISemaphore.d

./obj/hazelcast/client/HazelcastException.o: hazelcast/client/HazelcastException.cpp
	echo "Compiling hazelcast/client/HazelcastException.cpp  to ./obj/hazelcast/client/HazelcastException.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/HazelcastException.cpp -o ./obj/hazelcast/client/HazelcastException.o
	$(CC) $(CFLAGS) -MM hazelcast/client/HazelcastException.cpp > hazelcast/client/HazelcastException.d

./obj/hazelcast/client/serialization/BufferedDataInput.o: hazelcast/client/serialization/BufferedDataInput.cpp
	echo "Compiling hazelcast/client/serialization/BufferedDataInput.cpp  to ./obj/hazelcast/client/serialization/BufferedDataInput.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/BufferedDataInput.cpp -o ./obj/hazelcast/client/serialization/BufferedDataInput.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/BufferedDataInput.cpp > hazelcast/client/serialization/BufferedDataInput.d

./obj/hazelcast/client/serialization/PortableContext.o: hazelcast/client/serialization/PortableContext.cpp
	echo "Compiling hazelcast/client/serialization/PortableContext.cpp  to ./obj/hazelcast/client/serialization/PortableContext.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/PortableContext.cpp -o ./obj/hazelcast/client/serialization/PortableContext.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/PortableContext.cpp > hazelcast/client/serialization/PortableContext.d

./obj/hazelcast/client/serialization/PortableWriter.o: hazelcast/client/serialization/PortableWriter.cpp
	echo "Compiling hazelcast/client/serialization/PortableWriter.cpp  to ./obj/hazelcast/client/serialization/PortableWriter.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/PortableWriter.cpp -o ./obj/hazelcast/client/serialization/PortableWriter.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/PortableWriter.cpp > hazelcast/client/serialization/PortableWriter.d

./obj/hazelcast/client/impl/AbstractLoadBalancer.o: hazelcast/client/impl/AbstractLoadBalancer.cpp
	echo "Compiling hazelcast/client/impl/AbstractLoadBalancer.cpp  to ./obj/hazelcast/client/impl/AbstractLoadBalancer.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/impl/AbstractLoadBalancer.cpp -o ./obj/hazelcast/client/impl/AbstractLoadBalancer.o
	$(CC) $(CFLAGS) -MM hazelcast/client/impl/AbstractLoadBalancer.cpp > hazelcast/client/impl/AbstractLoadBalancer.d

./obj/hazelcast/client/serialization/SerializationService.o: hazelcast/client/serialization/SerializationService.cpp
	echo "Compiling hazelcast/client/serialization/SerializationService.cpp  to ./obj/hazelcast/client/serialization/SerializationService.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/SerializationService.cpp -o ./obj/hazelcast/client/serialization/SerializationService.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/SerializationService.cpp > hazelcast/client/serialization/SerializationService.d

./obj/hazelcast/client/serialization/PortableReader.o: hazelcast/client/serialization/PortableReader.cpp
	echo "Compiling hazelcast/client/serialization/PortableReader.cpp  to ./obj/hazelcast/client/serialization/PortableReader.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/PortableReader.cpp -o ./obj/hazelcast/client/serialization/PortableReader.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/PortableReader.cpp > hazelcast/client/serialization/PortableReader.d

./obj/hazelcast/client/IdGenerator.o: hazelcast/client/IdGenerator.cpp
	echo "Compiling hazelcast/client/IdGenerator.cpp  to ./obj/hazelcast/client/IdGenerator.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/IdGenerator.cpp -o ./obj/hazelcast/client/IdGenerator.o
	$(CC) $(CFLAGS) -MM hazelcast/client/IdGenerator.cpp > hazelcast/client/IdGenerator.d

./obj/hazelcast/client/serialization/FieldDefinition.o: hazelcast/client/serialization/FieldDefinition.cpp
	echo "Compiling hazelcast/client/serialization/FieldDefinition.cpp  to ./obj/hazelcast/client/serialization/FieldDefinition.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/FieldDefinition.cpp -o ./obj/hazelcast/client/serialization/FieldDefinition.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/FieldDefinition.cpp > hazelcast/client/serialization/FieldDefinition.d

./obj/hazelcast/client/map/RemoveRequest.o: hazelcast/client/map/RemoveRequest.cpp
	echo "Compiling hazelcast/client/map/RemoveRequest.cpp  to ./obj/hazelcast/client/map/RemoveRequest.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/map/RemoveRequest.cpp -o ./obj/hazelcast/client/map/RemoveRequest.o
	$(CC) $(CFLAGS) -MM hazelcast/client/map/RemoveRequest.cpp > hazelcast/client/map/RemoveRequest.d

./obj/hazelcast/client/connection/HeartBeatChecker.o: hazelcast/client/connection/HeartBeatChecker.cpp
	echo "Compiling hazelcast/client/connection/HeartBeatChecker.cpp  to ./obj/hazelcast/client/connection/HeartBeatChecker.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/HeartBeatChecker.cpp -o ./obj/hazelcast/client/connection/HeartBeatChecker.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/HeartBeatChecker.cpp > hazelcast/client/connection/HeartBeatChecker.d

./obj/hazelcast/client/spi/ClusterService.o: hazelcast/client/spi/ClusterService.cpp
	echo "Compiling hazelcast/client/spi/ClusterService.cpp  to ./obj/hazelcast/client/spi/ClusterService.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/spi/ClusterService.cpp -o ./obj/hazelcast/client/spi/ClusterService.o
	$(CC) $(CFLAGS) -MM hazelcast/client/spi/ClusterService.cpp > hazelcast/client/spi/ClusterService.d

./obj/hazelcast/client/connection/Member.o: hazelcast/client/connection/Member.cpp
	echo "Compiling hazelcast/client/connection/Member.cpp  to ./obj/hazelcast/client/connection/Member.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/Member.cpp -o ./obj/hazelcast/client/connection/Member.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/Member.cpp > hazelcast/client/connection/Member.d

./obj/hazelcast/client/ClientConfig.o: hazelcast/client/ClientConfig.cpp
	echo "Compiling hazelcast/client/ClientConfig.cpp  to ./obj/hazelcast/client/ClientConfig.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/ClientConfig.cpp -o ./obj/hazelcast/client/ClientConfig.o
	$(CC) $(CFLAGS) -MM hazelcast/client/ClientConfig.cpp > hazelcast/client/ClientConfig.d

./obj/hazelcast/client/serialization/Data.o: hazelcast/client/serialization/Data.cpp
	echo "Compiling hazelcast/client/serialization/Data.cpp  to ./obj/hazelcast/client/serialization/Data.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/Data.cpp -o ./obj/hazelcast/client/serialization/Data.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/Data.cpp > hazelcast/client/serialization/Data.d

./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.o: hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.cpp
	echo "Compiling hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.cpp  to ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.cpp -o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.cpp > hazelcast/client/serialization/ConstantSerializers/ConstantsPortableReader.d

./obj/hazelcast/client/ICountDownLatch.o: hazelcast/client/ICountDownLatch.cpp
	echo "Compiling hazelcast/client/ICountDownLatch.cpp  to ./obj/hazelcast/client/ICountDownLatch.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/ICountDownLatch.cpp -o ./obj/hazelcast/client/ICountDownLatch.o
	$(CC) $(CFLAGS) -MM hazelcast/client/ICountDownLatch.cpp > hazelcast/client/ICountDownLatch.d

./obj/hazelcast/client/protocol/ClientPingRequest.o: hazelcast/client/protocol/ClientPingRequest.cpp
	echo "Compiling hazelcast/client/protocol/ClientPingRequest.cpp  to ./obj/hazelcast/client/protocol/ClientPingRequest.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/protocol/ClientPingRequest.cpp -o ./obj/hazelcast/client/protocol/ClientPingRequest.o
	$(CC) $(CFLAGS) -MM hazelcast/client/protocol/ClientPingRequest.cpp > hazelcast/client/protocol/ClientPingRequest.d

./obj/hazelcast/client/serialization/ClassDefinitionWriter.o: hazelcast/client/serialization/ClassDefinitionWriter.cpp
	echo "Compiling hazelcast/client/serialization/ClassDefinitionWriter.cpp  to ./obj/hazelcast/client/serialization/ClassDefinitionWriter.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ClassDefinitionWriter.cpp -o ./obj/hazelcast/client/serialization/ClassDefinitionWriter.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ClassDefinitionWriter.cpp > hazelcast/client/serialization/ClassDefinitionWriter.d

./obj/hazelcast/client/HazelcastClient.o: hazelcast/client/HazelcastClient.cpp
	echo "Compiling hazelcast/client/HazelcastClient.cpp  to ./obj/hazelcast/client/HazelcastClient.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/HazelcastClient.cpp -o ./obj/hazelcast/client/HazelcastClient.o
	$(CC) $(CFLAGS) -MM hazelcast/client/HazelcastClient.cpp > hazelcast/client/HazelcastClient.d

./obj/hazelcast/client/protocol/SocketPool.o: hazelcast/client/protocol/SocketPool.cpp
	echo "Compiling hazelcast/client/protocol/SocketPool.cpp  to ./obj/hazelcast/client/protocol/SocketPool.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/protocol/SocketPool.cpp -o ./obj/hazelcast/client/protocol/SocketPool.o
	$(CC) $(CFLAGS) -MM hazelcast/client/protocol/SocketPool.cpp > hazelcast/client/protocol/SocketPool.d

./obj/hazelcast/client/connection/Connection.o: hazelcast/client/connection/Connection.cpp
	echo "Compiling hazelcast/client/connection/Connection.cpp  to ./obj/hazelcast/client/connection/Connection.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/Connection.cpp -o ./obj/hazelcast/client/connection/Connection.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/Connection.cpp > hazelcast/client/connection/Connection.d

./obj/hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.o: hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.cpp
	echo "Compiling hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.cpp  to ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.cpp -o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.cpp > hazelcast/client/serialization/ConstantSerializers/ConstantPortableWriter.d

./obj/hazelcast/client/serialization/NullPortable.o: hazelcast/client/serialization/NullPortable.cpp
	echo "Compiling hazelcast/client/serialization/NullPortable.cpp  to ./obj/hazelcast/client/serialization/NullPortable.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/NullPortable.cpp -o ./obj/hazelcast/client/serialization/NullPortable.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/NullPortable.cpp > hazelcast/client/serialization/NullPortable.d

./obj/hazelcast/client/protocol/AuthenticationRequest.o: hazelcast/client/protocol/AuthenticationRequest.cpp
	echo "Compiling hazelcast/client/protocol/AuthenticationRequest.cpp  to ./obj/hazelcast/client/protocol/AuthenticationRequest.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/protocol/AuthenticationRequest.cpp -o ./obj/hazelcast/client/protocol/AuthenticationRequest.o
	$(CC) $(CFLAGS) -MM hazelcast/client/protocol/AuthenticationRequest.cpp > hazelcast/client/protocol/AuthenticationRequest.d

./obj/hazelcast/client/protocol/HazelcastServerError.o: hazelcast/client/protocol/HazelcastServerError.cpp
	echo "Compiling hazelcast/client/protocol/HazelcastServerError.cpp  to ./obj/hazelcast/client/protocol/HazelcastServerError.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/protocol/HazelcastServerError.cpp -o ./obj/hazelcast/client/protocol/HazelcastServerError.o
	$(CC) $(CFLAGS) -MM hazelcast/client/protocol/HazelcastServerError.cpp > hazelcast/client/protocol/HazelcastServerError.d

./obj/hazelcast/util/ConcurrentQueue.o: hazelcast/util/ConcurrentQueue.cpp
	echo "Compiling hazelcast/util/ConcurrentQueue.cpp  to ./obj/hazelcast/util/ConcurrentQueue.o ..."
	$(CC) $(CFLAGS) -c hazelcast/util/ConcurrentQueue.cpp -o ./obj/hazelcast/util/ConcurrentQueue.o
	$(CC) $(CFLAGS) -MM hazelcast/util/ConcurrentQueue.cpp > hazelcast/util/ConcurrentQueue.d

./obj/hazelcast/client/connection/Socket.o: hazelcast/client/connection/Socket.cpp
	echo "Compiling hazelcast/client/connection/Socket.cpp  to ./obj/hazelcast/client/connection/Socket.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/Socket.cpp -o ./obj/hazelcast/client/connection/Socket.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/Socket.cpp > hazelcast/client/connection/Socket.d

./obj/hazelcast/client/serialization/BufferedDataOutput.o: hazelcast/client/serialization/BufferedDataOutput.cpp
	echo "Compiling hazelcast/client/serialization/BufferedDataOutput.cpp  to ./obj/hazelcast/client/serialization/BufferedDataOutput.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/BufferedDataOutput.cpp -o ./obj/hazelcast/client/serialization/BufferedDataOutput.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/BufferedDataOutput.cpp > hazelcast/client/serialization/BufferedDataOutput.d

./obj/hazelcast/client/serialization/InputSocketStream.o: hazelcast/client/serialization/InputSocketStream.cpp
	echo "Compiling hazelcast/client/serialization/InputSocketStream.cpp  to ./obj/hazelcast/client/serialization/InputSocketStream.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/InputSocketStream.cpp -o ./obj/hazelcast/client/serialization/InputSocketStream.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/InputSocketStream.cpp > hazelcast/client/serialization/InputSocketStream.d

./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.o: hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.cpp
	echo "Compiling hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.cpp  to ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.cpp -o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.cpp > hazelcast/client/serialization/ConstantSerializers/ConstantsMorphingPortableReader.d

./obj/hazelcast/client/serialization/MorphingPortableReader.o: hazelcast/client/serialization/MorphingPortableReader.cpp
	echo "Compiling hazelcast/client/serialization/MorphingPortableReader.cpp  to ./obj/hazelcast/client/serialization/MorphingPortableReader.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/MorphingPortableReader.cpp -o ./obj/hazelcast/client/serialization/MorphingPortableReader.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/MorphingPortableReader.cpp > hazelcast/client/serialization/MorphingPortableReader.d

./obj/hazelcast/client/map/PutRequest.o: hazelcast/client/map/PutRequest.cpp
	echo "Compiling hazelcast/client/map/PutRequest.cpp  to ./obj/hazelcast/client/map/PutRequest.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/map/PutRequest.cpp -o ./obj/hazelcast/client/map/PutRequest.o
	$(CC) $(CFLAGS) -MM hazelcast/client/map/PutRequest.cpp > hazelcast/client/map/PutRequest.d

./obj/hazelcast/client/map/GetRequest.o: hazelcast/client/map/GetRequest.cpp
	echo "Compiling hazelcast/client/map/GetRequest.cpp  to ./obj/hazelcast/client/map/GetRequest.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/map/GetRequest.cpp -o ./obj/hazelcast/client/map/GetRequest.o
	$(CC) $(CFLAGS) -MM hazelcast/client/map/GetRequest.cpp > hazelcast/client/map/GetRequest.d

./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.o: hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.cpp
	echo "Compiling hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.cpp  to ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.cpp -o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.cpp > hazelcast/client/serialization/ConstantSerializers/ConstantDataOutput.d

./obj/hazelcast/client/IAtomicLong.o: hazelcast/client/IAtomicLong.cpp
	echo "Compiling hazelcast/client/IAtomicLong.cpp  to ./obj/hazelcast/client/IAtomicLong.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/IAtomicLong.cpp -o ./obj/hazelcast/client/IAtomicLong.o
	$(CC) $(CFLAGS) -MM hazelcast/client/IAtomicLong.cpp > hazelcast/client/IAtomicLong.d

./obj/hazelcast/util/Lock.o: hazelcast/util/Lock.cpp
	echo "Compiling hazelcast/util/Lock.cpp  to ./obj/hazelcast/util/Lock.o ..."
	$(CC) $(CFLAGS) -c hazelcast/util/Lock.cpp -o ./obj/hazelcast/util/Lock.o
	$(CC) $(CFLAGS) -MM hazelcast/util/Lock.cpp > hazelcast/util/Lock.d

./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o: hazelcast/client/serialization/ClassDefinitionBuilder.cpp
	echo "Compiling hazelcast/client/serialization/ClassDefinitionBuilder.cpp  to ./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ClassDefinitionBuilder.cpp -o ./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ClassDefinitionBuilder.cpp > hazelcast/client/serialization/ClassDefinitionBuilder.d

./obj/hazelcast/client/Address.o: hazelcast/client/Address.cpp
	echo "Compiling hazelcast/client/Address.cpp  to ./obj/hazelcast/client/Address.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/Address.cpp -o ./obj/hazelcast/client/Address.o
	$(CC) $(CFLAGS) -MM hazelcast/client/Address.cpp > hazelcast/client/Address.d

./obj/hazelcast/client/serialization/DataSerializer.o: hazelcast/client/serialization/DataSerializer.cpp
	echo "Compiling hazelcast/client/serialization/DataSerializer.cpp  to ./obj/hazelcast/client/serialization/DataSerializer.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/DataSerializer.cpp -o ./obj/hazelcast/client/serialization/DataSerializer.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/DataSerializer.cpp > hazelcast/client/serialization/DataSerializer.d

./obj/hazelcast/util/Util.o: hazelcast/util/Util.cpp
	echo "Compiling hazelcast/util/Util.cpp  to ./obj/hazelcast/util/Util.o ..."
	$(CC) $(CFLAGS) -c hazelcast/util/Util.cpp -o ./obj/hazelcast/util/Util.o
	$(CC) $(CFLAGS) -MM hazelcast/util/Util.cpp > hazelcast/util/Util.d

./obj/hazelcast/util/ConcurrentMap.o: hazelcast/util/ConcurrentMap.cpp
	echo "Compiling hazelcast/util/ConcurrentMap.cpp  to ./obj/hazelcast/util/ConcurrentMap.o ..."
	$(CC) $(CFLAGS) -c hazelcast/util/ConcurrentMap.cpp -o ./obj/hazelcast/util/ConcurrentMap.o
	$(CC) $(CFLAGS) -MM hazelcast/util/ConcurrentMap.cpp > hazelcast/util/ConcurrentMap.d

./obj/hazelcast/client/Cluster.o: hazelcast/client/Cluster.cpp
	echo "Compiling hazelcast/client/Cluster.cpp  to ./obj/hazelcast/client/Cluster.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/Cluster.cpp -o ./obj/hazelcast/client/Cluster.o
	$(CC) $(CFLAGS) -MM hazelcast/client/Cluster.cpp > hazelcast/client/Cluster.d

./obj/hazelcast/client/spi/ClientContext.o: hazelcast/client/spi/ClientContext.cpp
	echo "Compiling hazelcast/client/spi/ClientContext.cpp  to ./obj/hazelcast/client/spi/ClientContext.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/spi/ClientContext.cpp -o ./obj/hazelcast/client/spi/ClientContext.o
	$(CC) $(CFLAGS) -MM hazelcast/client/spi/ClientContext.cpp > hazelcast/client/spi/ClientContext.d

./obj/hazelcast/client/connection/MembershipEvent.o: hazelcast/client/connection/MembershipEvent.cpp
	echo "Compiling hazelcast/client/connection/MembershipEvent.cpp  to ./obj/hazelcast/client/connection/MembershipEvent.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/MembershipEvent.cpp -o ./obj/hazelcast/client/connection/MembershipEvent.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/MembershipEvent.cpp > hazelcast/client/connection/MembershipEvent.d

./obj/hazelcast/client/protocol/Principal.o: hazelcast/client/protocol/Principal.cpp
	echo "Compiling hazelcast/client/protocol/Principal.cpp  to ./obj/hazelcast/client/protocol/Principal.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/protocol/Principal.cpp -o ./obj/hazelcast/client/protocol/Principal.o
	$(CC) $(CFLAGS) -MM hazelcast/client/protocol/Principal.cpp > hazelcast/client/protocol/Principal.d

./obj/hazelcast/client/connection/ConnectionManager.o: hazelcast/client/connection/ConnectionManager.cpp
	echo "Compiling hazelcast/client/connection/ConnectionManager.cpp  to ./obj/hazelcast/client/connection/ConnectionManager.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/ConnectionManager.cpp -o ./obj/hazelcast/client/connection/ConnectionManager.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/ConnectionManager.cpp > hazelcast/client/connection/ConnectionManager.d

./obj/hazelcast/client/serialization/OutputSocketStream.o: hazelcast/client/serialization/OutputSocketStream.cpp
	echo "Compiling hazelcast/client/serialization/OutputSocketStream.cpp  to ./obj/hazelcast/client/serialization/OutputSocketStream.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/OutputSocketStream.cpp -o ./obj/hazelcast/client/serialization/OutputSocketStream.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/OutputSocketStream.cpp > hazelcast/client/serialization/OutputSocketStream.d

./obj/hazelcast/client/serialization/ClassDefinition.o: hazelcast/client/serialization/ClassDefinition.cpp
	echo "Compiling hazelcast/client/serialization/ClassDefinition.cpp  to ./obj/hazelcast/client/serialization/ClassDefinition.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ClassDefinition.cpp -o ./obj/hazelcast/client/serialization/ClassDefinition.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ClassDefinition.cpp > hazelcast/client/serialization/ClassDefinition.d

./obj/hazelcast/client/spi/InvocationService.o: hazelcast/client/spi/InvocationService.cpp
	echo "Compiling hazelcast/client/spi/InvocationService.cpp  to ./obj/hazelcast/client/spi/InvocationService.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/spi/InvocationService.cpp -o ./obj/hazelcast/client/spi/InvocationService.o
	$(CC) $(CFLAGS) -MM hazelcast/client/spi/InvocationService.cpp > hazelcast/client/spi/InvocationService.d

./obj/main.o: main.cpp
	echo "Compiling main.cpp  to ./obj/main.o ..."
	$(CC) $(CFLAGS) -c main.cpp -o ./obj/main.o
	$(CC) $(CFLAGS) -MM main.cpp > main.d

./obj/hazelcast/client/connection/ConnectionPool.o: hazelcast/client/connection/ConnectionPool.cpp
	echo "Compiling hazelcast/client/connection/ConnectionPool.cpp  to ./obj/hazelcast/client/connection/ConnectionPool.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/connection/ConnectionPool.cpp -o ./obj/hazelcast/client/connection/ConnectionPool.o
	$(CC) $(CFLAGS) -MM hazelcast/client/connection/ConnectionPool.cpp > hazelcast/client/connection/ConnectionPool.d

./obj/hazelcast/client/serialization/SerializationContext.o: hazelcast/client/serialization/SerializationContext.cpp
	echo "Compiling hazelcast/client/serialization/SerializationContext.cpp  to ./obj/hazelcast/client/serialization/SerializationContext.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/SerializationContext.cpp -o ./obj/hazelcast/client/serialization/SerializationContext.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/SerializationContext.cpp > hazelcast/client/serialization/SerializationContext.d

./obj/hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.o: hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.cpp
	echo "Compiling hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.cpp  to ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.cpp -o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.cpp > hazelcast/client/serialization/ConstantSerializers/ConstantClassDefinitionWriter.d

./obj/hazelcast/client/impl/RoundRobinLB.o: hazelcast/client/impl/RoundRobinLB.cpp
	echo "Compiling hazelcast/client/impl/RoundRobinLB.cpp  to ./obj/hazelcast/client/impl/RoundRobinLB.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/impl/RoundRobinLB.cpp -o ./obj/hazelcast/client/impl/RoundRobinLB.o
	$(CC) $(CFLAGS) -MM hazelcast/client/impl/RoundRobinLB.cpp > hazelcast/client/impl/RoundRobinLB.d

./obj/hazelcast/util/SerializableCollection.o: hazelcast/util/SerializableCollection.cpp
	echo "Compiling hazelcast/util/SerializableCollection.cpp  to ./obj/hazelcast/util/SerializableCollection.o ..."
	$(CC) $(CFLAGS) -c hazelcast/util/SerializableCollection.cpp -o ./obj/hazelcast/util/SerializableCollection.o
	$(CC) $(CFLAGS) -MM hazelcast/util/SerializableCollection.cpp > hazelcast/util/SerializableCollection.d

./obj/hazelcast/client/protocol/AddMembershipListenerRequest.o: hazelcast/client/protocol/AddMembershipListenerRequest.cpp
	echo "Compiling hazelcast/client/protocol/AddMembershipListenerRequest.cpp  to ./obj/hazelcast/client/protocol/AddMembershipListenerRequest.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/protocol/AddMembershipListenerRequest.cpp -o ./obj/hazelcast/client/protocol/AddMembershipListenerRequest.o
	$(CC) $(CFLAGS) -MM hazelcast/client/protocol/AddMembershipListenerRequest.cpp > hazelcast/client/protocol/AddMembershipListenerRequest.d

./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.o: hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.cpp
	echo "Compiling hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.cpp  to ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.cpp -o ./obj/hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.cpp > hazelcast/client/serialization/ConstantSerializers/ConstantDataInput.d

./obj/hazelcast/client/GroupConfig.o: hazelcast/client/GroupConfig.cpp
	echo "Compiling hazelcast/client/GroupConfig.cpp  to ./obj/hazelcast/client/GroupConfig.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/GroupConfig.cpp -o ./obj/hazelcast/client/GroupConfig.o
	$(CC) $(CFLAGS) -MM hazelcast/client/GroupConfig.cpp > hazelcast/client/GroupConfig.d

./obj/hazelcast/client/serialization/PortableSerializer.o: hazelcast/client/serialization/PortableSerializer.cpp
	echo "Compiling hazelcast/client/serialization/PortableSerializer.cpp  to ./obj/hazelcast/client/serialization/PortableSerializer.o ..."
	$(CC) $(CFLAGS) -c hazelcast/client/serialization/PortableSerializer.cpp -o ./obj/hazelcast/client/serialization/PortableSerializer.o
	$(CC) $(CFLAGS) -MM hazelcast/client/serialization/PortableSerializer.cpp > hazelcast/client/serialization/PortableSerializer.d

clean:
	$(RM) $(OUT)
	$(RM) $(OBJS)

.SILENT:
