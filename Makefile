CC=clang++
OUT=hazelcastClient.out
RM=rm -f

OBJS=./obj/hazelcast/client/protocol/Credentials.o ./obj/hazelcast/client/connection/ClusterListenerThread.o ./obj/hazelcast/client/serialization/ConstantSerializers.o ./obj/hazelcast/client/spi/ClientService.o ./obj/hazelcast/client/util/Util.o ./obj/hazelcast/client/ISemaphore.o ./obj/hazelcast/client/HazelcastException.o ./obj/hazelcast/client/serialization/BufferedDataInput.o ./obj/hazelcast/client/serialization/PortableContext.o ./obj/hazelcast/client/serialization/PortableWriter.o ./obj/hazelcast/client/IdGenerator.o ./obj/hazelcast/client/serialization/FieldDefinition.o ./obj/hazelcast/client/serialization/PortableReader.o ./obj/hazelcast/client/serialization/SerializationService.o ./obj/hazelcast/client/map/RemoveRequest.o ./obj/hazelcast/client/connection/HeartBeatChecker.o ./obj/hazelcast/client/spi/ClusterService.o ./obj/hazelcast/client/ClientConfig.o ./obj/hazelcast/client/util/ConcurrentMap.o ./obj/hazelcast/client/serialization/Data.o ./obj/hazelcast/client/ICountDownLatch.o ./obj/hazelcast/client/util/ConcurrentQueue.o ./obj/hazelcast/client/protocol/ClientPingRequest.o ./obj/hazelcast/client/serialization/ClassDefinitionWriter.o ./obj/hazelcast/client/HazelcastClient.o ./obj/hazelcast/client/protocol/SocketPool.o ./obj/hazelcast/client/connection/Connection.o ./obj/hazelcast/client/serialization/NullPortable.o ./obj/hazelcast/client/protocol/AuthenticationRequest.o ./obj/hazelcast/client/protocol/HazelcastServerError.o ./obj/hazelcast/client/connection/Socket.o ./obj/hazelcast/client/serialization/BufferedDataOutput.o ./obj/hazelcast/client/serialization/InputSocketStream.o ./obj/hazelcast/client/serialization/MorphingPortableReader.o ./obj/hazelcast/client/map/PutRequest.o ./obj/hazelcast/client/map/GetRequest.o ./obj/hazelcast/client/IAtomicLong.o ./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o ./obj/hazelcast/client/Address.o ./obj/hazelcast/client/serialization/DataSerializer.o ./obj/hazelcast/client/spi/ClientContext.o ./obj/hazelcast/client/protocol/Principal.o ./obj/hazelcast/client/connection/ConnectionManager.o ./obj/hazelcast/client/serialization/ClassDefinition.o ./obj/hazelcast/client/serialization/OutputSocketStream.o ./obj/hazelcast/client/spi/InvocationService.o ./obj/main.o ./obj/hazelcast/client/connection/ConnectionPool.o ./obj/hazelcast/client/util/QueueBasedObjectPool.o ./obj/hazelcast/client/serialization/SerializationContext.o ./obj/hazelcast/client/GroupConfig.o ./obj/hazelcast/client/serialization/PortableSerializer.o 
CFLAGS=
LFLAGS= /usr/local/lib/libz.a /usr/local/lib/libboost_thread.a /usr/local/lib/libboost_system.a

all: $(OUT)
	echo "Finished."

$(OUT): $(OBJS)
	echo "Linking $(OUT)..."
	$(CC)  $(OBJS) -o $(OUT) $(LFLAGS)

./obj/hazelcast/client/protocol/Credentials.o: ./hazelcast/client/protocol/Credentials.cpp ./hazelcast/client/protocol/Credentials.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/connection/ClusterListenerThread.o: ./hazelcast/client/connection/ClusterListenerThread.cpp ./hazelcast/client/connection/ClusterListenerThread.h ./hazelcast/client/connection/Connection.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ConstantSerializers.o: ./hazelcast/client/serialization/ConstantSerializers.cpp ./hazelcast/client/serialization/ConstantSerializers.h ./hazelcast/client/serialization/ClassDefinitionWriter.h ./hazelcast/client/serialization/MorphingPortableReader.h ./hazelcast/client/serialization/BufferedDataOutput.h ./hazelcast/client/serialization/PortableWriter.h ./hazelcast/client/serialization/Data.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/spi/ClientService.o: ./hazelcast/client/spi/ClientService.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/util/Util.o: ./hazelcast/client/util/Util.cpp ./hazelcast/client/util/Util.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ISemaphore.o: ./hazelcast/client/ISemaphore.cpp ./hazelcast/client/ISemaphore.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/HazelcastException.o: ./hazelcast/client/HazelcastException.cpp ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/BufferedDataInput.o: ./hazelcast/client/serialization/BufferedDataInput.cpp ./hazelcast/client/serialization/BufferedDataInput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableContext.o: ./hazelcast/client/serialization/PortableContext.cpp ./hazelcast/client/serialization/PortableContext.h ./hazelcast/client/serialization/BufferedDataInput.h ./hazelcast/client/serialization/SerializationService.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableWriter.o: ./hazelcast/client/serialization/PortableWriter.cpp ./hazelcast/client/serialization/PortableWriter.h ./hazelcast/client/util/Util.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/IdGenerator.o: ./hazelcast/client/IdGenerator.cpp ./hazelcast/client/IdGenerator.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/FieldDefinition.o: ./hazelcast/client/serialization/FieldDefinition.cpp ./hazelcast/client/serialization/FieldDefinition.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableReader.o: ./hazelcast/client/serialization/PortableReader.cpp ./hazelcast/client/serialization/SerializationContext.h ./hazelcast/client/serialization/PortableReader.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationService.o: ./hazelcast/client/serialization/SerializationService.cpp ./hazelcast/client/serialization/SerializationService.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/map/RemoveRequest.o: ./hazelcast/client/map/RemoveRequest.cpp ./hazelcast/client/map/RemoveRequest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/connection/HeartBeatChecker.o: ./hazelcast/client/connection/HeartBeatChecker.cpp ./hazelcast/client/connection/HeartBeatChecker.h ./hazelcast/client/connection/Connection.h ./hazelcast/client/protocol/ClientPingRequest.h ./hazelcast/client/serialization/SerializationService.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/spi/ClusterService.o: ./hazelcast/client/spi/ClusterService.cpp ./hazelcast/client/spi/ClusterService.h ./hazelcast/client/ClientConfig.h ./hazelcast/client/HazelcastClient.h ./hazelcast/client/serialization/ClassDefinitionBuilder.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ClientConfig.o: ./hazelcast/client/ClientConfig.cpp ./hazelcast/client/ClientConfig.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/util/ConcurrentMap.o: ./hazelcast/client/util/ConcurrentMap.cpp ./hazelcast/client/util/ConcurrentMap.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/Data.o: ./hazelcast/client/serialization/Data.cpp ./hazelcast/client/serialization/Data.h ./hazelcast/client/serialization/BufferedDataOutput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ICountDownLatch.o: ./hazelcast/client/ICountDownLatch.cpp ./hazelcast/client/ICountDownLatch.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/util/ConcurrentQueue.o: ./hazelcast/client/util/ConcurrentQueue.cpp ./hazelcast/client/util/ConcurrentQueue.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/ClientPingRequest.o: ./hazelcast/client/protocol/ClientPingRequest.cpp ./hazelcast/client/protocol/ClientPingRequest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinitionWriter.o: ./hazelcast/client/serialization/ClassDefinitionWriter.cpp ./hazelcast/client/serialization/ClassDefinitionWriter.h ./hazelcast/client/serialization/PortableSerializer.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/HazelcastClient.o: ./hazelcast/client/HazelcastClient.cpp ./hazelcast/client/HazelcastClient.h ./hazelcast/client/IdGenerator.h ./hazelcast/client/IAtomicLong.h ./hazelcast/client/ICountDownLatch.h ./hazelcast/client/ISemaphore.h ./hazelcast/client/ClientConfig.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/SocketPool.o: ./hazelcast/client/protocol/SocketPool.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/connection/Connection.o: ./hazelcast/client/connection/Connection.cpp ./hazelcast/client/connection/Connection.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/NullPortable.o: ./hazelcast/client/serialization/NullPortable.cpp ./hazelcast/client/serialization/NullPortable.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/AuthenticationRequest.o: ./hazelcast/client/protocol/AuthenticationRequest.cpp ./hazelcast/client/protocol/AuthenticationRequest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/HazelcastServerError.o: ./hazelcast/client/protocol/HazelcastServerError.cpp ./hazelcast/client/protocol/HazelcastServerError.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/connection/Socket.o: ./hazelcast/client/connection/Socket.cpp ./hazelcast/client/connection/Socket.h ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/BufferedDataOutput.o: ./hazelcast/client/serialization/BufferedDataOutput.cpp ./hazelcast/client/serialization/BufferedDataOutput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/InputSocketStream.o: ./hazelcast/client/serialization/InputSocketStream.cpp ./hazelcast/client/serialization/InputSocketStream.h ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/MorphingPortableReader.o: ./hazelcast/client/serialization/MorphingPortableReader.cpp ./hazelcast/client/serialization/MorphingPortableReader.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/map/PutRequest.o: ./hazelcast/client/map/PutRequest.cpp ./hazelcast/client/map/PutRequest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/map/GetRequest.o: ./hazelcast/client/map/GetRequest.cpp ./hazelcast/client/map/GetRequest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/IAtomicLong.o: ./hazelcast/client/IAtomicLong.cpp ./hazelcast/client/IAtomicLong.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o: ./hazelcast/client/serialization/ClassDefinitionBuilder.cpp ./hazelcast/client/serialization/ClassDefinitionBuilder.h ./hazelcast/client/HazelcastException.h ./hazelcast/client/util/Util.h ./hazelcast/client/serialization/Data.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/Address.o: ./hazelcast/client/Address.cpp ./hazelcast/client/Address.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataSerializer.o: ./hazelcast/client/serialization/DataSerializer.cpp ./hazelcast/client/serialization/DataSerializer.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/spi/ClientContext.o: ./hazelcast/client/spi/ClientContext.cpp ./hazelcast/client/spi/ClientContext.h ./hazelcast/client/HazelcastClient.h ./hazelcast/client/ClientConfig.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/Principal.o: ./hazelcast/client/protocol/Principal.cpp ./hazelcast/client/protocol/Principal.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/connection/ConnectionManager.o: ./hazelcast/client/connection/ConnectionManager.cpp ./hazelcast/client/connection/ConnectionManager.h ./hazelcast/client/ClientConfig.h ./hazelcast/client/connection/Connection.h ./hazelcast/client/protocol/ClientPingRequest.h ./hazelcast/client/protocol/AuthenticationRequest.h ./hazelcast/client/protocol/HazelcastServerError.h ./hazelcast/client/serialization/SerializationService.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinition.o: ./hazelcast/client/serialization/ClassDefinition.cpp ./hazelcast/client/serialization/ClassDefinition.h ./hazelcast/client/serialization/BufferedDataInput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/OutputSocketStream.o: ./hazelcast/client/serialization/OutputSocketStream.cpp ./hazelcast/client/serialization/OutputSocketStream.h ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/spi/InvocationService.o: ./hazelcast/client/spi/InvocationService.cpp ./hazelcast/client/spi/InvocationService.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/main.o: ./main.cpp ./TestDataSerializable.h ./portableTest.h ./mapTest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/connection/ConnectionPool.o: ./hazelcast/client/connection/ConnectionPool.cpp ./hazelcast/client/connection/ConnectionPool.h ./hazelcast/client/Address.h ./hazelcast/client/connection/Connection.h ./hazelcast/client/connection/ConnectionManager.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/util/QueueBasedObjectPool.o: ./hazelcast/client/util/QueueBasedObjectPool.cpp ./hazelcast/client/util/QueueBasedObjectPool.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationContext.o: ./hazelcast/client/serialization/SerializationContext.cpp ./hazelcast/client/serialization/ClassDefinition.h ./hazelcast/client/serialization/SerializationContext.h ./hazelcast/client/serialization/SerializationService.h ./hazelcast/client/serialization/PortableContext.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/GroupConfig.o: ./hazelcast/client/GroupConfig.cpp ./hazelcast/client/GroupConfig.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableSerializer.o: ./hazelcast/client/serialization/PortableSerializer.cpp ./hazelcast/client/serialization/PortableSerializer.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OUT)
	$(RM) $(OBJS)

.SILENT:
