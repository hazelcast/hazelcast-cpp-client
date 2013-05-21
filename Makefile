CC=clang++
OUT=hazelcastClient.out
RM=rm -f

OBJS=./obj/hazelcast/client/protocol/AuthenticationRequest.o ./obj/hazelcast/client/protocol/HazelcastServerError.o ./obj/hazelcast/client/protocol/MapPutOperation.o ./obj/hazelcast/client/protocol/Credentials.o ./obj/hazelcast/client/serialization/BufferedDataOutput.o ./obj/hazelcast/client/serialization/InputSocketStream.o ./obj/hazelcast/client/serialization/ConstantSerializers.o ./obj/hazelcast/client/serialization/MorphingPortableReader.o ./obj/hazelcast/client/impl/ClientService.o ./obj/hazelcast/client/util/Util.o ./obj/hazelcast/client/ISemaphore.o ./obj/hazelcast/client/HazelcastException.o ./obj/hazelcast/client/IAtomicLong.o ./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o ./obj/hazelcast/client/serialization/BufferedDataInput.o ./obj/hazelcast/client/serialization/DataSerializer.o ./obj/hazelcast/client/Address.o ./obj/hazelcast/client/protocol/Socket.o ./obj/hazelcast/client/serialization/PortableContext.o ./obj/hazelcast/client/serialization/PortableWriter.o ./obj/hazelcast/client/serialization/SerializationService.o ./obj/hazelcast/client/serialization/PortableReader.o ./obj/hazelcast/client/serialization/FieldDefinition.o ./obj/hazelcast/client/IdGenerator.o ./obj/hazelcast/client/protocol/Principal.o ./obj/hazelcast/client/serialization/OutputSocketStream.o ./obj/hazelcast/client/serialization/ClassDefinition.o ./obj/hazelcast/client/ClientConfig.o ./obj/main.o ./obj/hazelcast/client/serialization/Data.o ./obj/hazelcast/client/serialization/SerializationContext.o ./obj/hazelcast/client/ICountDownLatch.o ./obj/hazelcast/client/serialization/ClassDefinitionWriter.o ./obj/hazelcast/client/GroupConfig.o ./obj/hazelcast/client/HazelcastClient.o ./obj/hazelcast/client/protocol/SocketPool.o ./obj/hazelcast/client/serialization/PortableSerializer.o ./obj/hazelcast/client/serialization/NullPortable.o 
CFLAGS=
LFLAGS= /usr/local/lib/libz.a /usr/local/lib/libboost_thread.a /usr/local/lib/libboost_system.a

all: $(OUT)
	echo "Finished."

$(OUT): $(OBJS)
	echo "Linking $(OUT)..."
	$(CC)  $(OBJS) -o $(OUT) $(LFLAGS)

./obj/hazelcast/client/protocol/AuthenticationRequest.o: ./hazelcast/client/protocol/AuthenticationRequest.cpp ./hazelcast/client/protocol/AuthenticationRequest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/HazelcastServerError.o: ./hazelcast/client/protocol/HazelcastServerError.cpp ./hazelcast/client/protocol/HazelcastServerError.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/MapPutOperation.o: ./hazelcast/client/protocol/MapPutOperation.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/Credentials.o: ./hazelcast/client/protocol/Credentials.cpp ./hazelcast/client/protocol/Credentials.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/BufferedDataOutput.o: ./hazelcast/client/serialization/BufferedDataOutput.cpp ./hazelcast/client/serialization/BufferedDataOutput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/InputSocketStream.o: ./hazelcast/client/serialization/InputSocketStream.cpp ./hazelcast/client/serialization/InputSocketStream.h ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ConstantSerializers.o: ./hazelcast/client/serialization/ConstantSerializers.cpp ./hazelcast/client/serialization/ConstantSerializers.h ./hazelcast/client/serialization/ClassDefinitionWriter.h ./hazelcast/client/serialization/MorphingPortableReader.h ./hazelcast/client/serialization/BufferedDataOutput.h ./hazelcast/client/serialization/PortableWriter.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/MorphingPortableReader.o: ./hazelcast/client/serialization/MorphingPortableReader.cpp ./hazelcast/client/serialization/MorphingPortableReader.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/impl/ClientService.o: ./hazelcast/client/impl/ClientService.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/util/Util.o: ./hazelcast/client/util/Util.cpp ./hazelcast/client/util/Util.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ISemaphore.o: ./hazelcast/client/ISemaphore.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/HazelcastException.o: ./hazelcast/client/HazelcastException.cpp ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/IAtomicLong.o: ./hazelcast/client/IAtomicLong.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinitionBuilder.o: ./hazelcast/client/serialization/ClassDefinitionBuilder.cpp ./hazelcast/client/serialization/ClassDefinitionBuilder.h ./hazelcast/client/HazelcastException.h ./hazelcast/client/util/Util.h ./hazelcast/client/serialization/Data.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/BufferedDataInput.o: ./hazelcast/client/serialization/BufferedDataInput.cpp ./hazelcast/client/serialization/BufferedDataInput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataSerializer.o: ./hazelcast/client/serialization/DataSerializer.cpp ./hazelcast/client/serialization/DataSerializer.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/Address.o: ./hazelcast/client/Address.cpp ./hazelcast/client/Address.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/Socket.o: ./hazelcast/client/protocol/Socket.cpp ./hazelcast/client/protocol/Socket.h ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableContext.o: ./hazelcast/client/serialization/PortableContext.cpp ./hazelcast/client/serialization/PortableContext.h ./hazelcast/client/serialization/BufferedDataInput.h ./hazelcast/client/serialization/SerializationService.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableWriter.o: ./hazelcast/client/serialization/PortableWriter.cpp ./hazelcast/client/serialization/PortableWriter.h ./hazelcast/client/util/Util.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationService.o: ./hazelcast/client/serialization/SerializationService.cpp ./hazelcast/client/serialization/SerializationService.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableReader.o: ./hazelcast/client/serialization/PortableReader.cpp ./hazelcast/client/serialization/SerializationContext.h ./hazelcast/client/serialization/PortableReader.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/FieldDefinition.o: ./hazelcast/client/serialization/FieldDefinition.cpp ./hazelcast/client/serialization/FieldDefinition.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/IdGenerator.o: ./hazelcast/client/IdGenerator.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/Principal.o: ./hazelcast/client/protocol/Principal.cpp ./hazelcast/client/protocol/Principal.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/OutputSocketStream.o: ./hazelcast/client/serialization/OutputSocketStream.cpp ./hazelcast/client/serialization/OutputSocketStream.h ./hazelcast/client/HazelcastException.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinition.o: ./hazelcast/client/serialization/ClassDefinition.cpp ./hazelcast/client/serialization/ClassDefinition.h ./hazelcast/client/serialization/BufferedDataInput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ClientConfig.o: ./hazelcast/client/ClientConfig.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/main.o: ./main.cpp ./TestDataSerializable.h ./portableTest.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/Data.o: ./hazelcast/client/serialization/Data.cpp ./hazelcast/client/serialization/Data.h ./hazelcast/client/serialization/BufferedDataOutput.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationContext.o: ./hazelcast/client/serialization/SerializationContext.cpp ./hazelcast/client/serialization/ClassDefinition.h ./hazelcast/client/serialization/SerializationContext.h ./hazelcast/client/serialization/SerializationService.h ./hazelcast/client/serialization/PortableContext.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ICountDownLatch.o: ./hazelcast/client/ICountDownLatch.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinitionWriter.o: ./hazelcast/client/serialization/ClassDefinitionWriter.cpp ./hazelcast/client/serialization/ClassDefinitionWriter.h ./hazelcast/client/serialization/PortableSerializer.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/GroupConfig.o: ./hazelcast/client/GroupConfig.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/HazelcastClient.o: ./hazelcast/client/HazelcastClient.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/SocketPool.o: ./hazelcast/client/protocol/SocketPool.cpp 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableSerializer.o: ./hazelcast/client/serialization/PortableSerializer.cpp ./hazelcast/client/serialization/PortableSerializer.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/NullPortable.o: ./hazelcast/client/serialization/NullPortable.cpp ./hazelcast/client/serialization/NullPortable.h 
	echo "Compiling $^ to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OUT)
	$(RM) $(OBJS)

.SILENT:
