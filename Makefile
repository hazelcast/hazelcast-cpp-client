CC=g++
OUT=p.out
RM=rm -f

OBJS=./obj/hazelcast/client/ISemaphore.o ./obj/hazelcast/client/ClientConfig.o ./obj/hazelcast/client/serialization/DataInput.o ./obj/hazelcast/client/serialization/Data.o ./obj/hazelcast/client/serialization/FieldDefinition.o ./obj/hazelcast/client/GroupConfig.o ./obj/hazelcast/client/serialization/PortableWriter.o ./obj/hazelcast/client/IdGenerator.o ./obj/hazelcast/client/serialization/ClassDefinition.o ./obj/hazelcast/client/Address.o ./obj/hazelcast/client/ClientService.o ./obj/hazelcast/client/serialization/PortableSerializer.o ./obj/hazelcast/client/HazelcastClient.o ./obj/hazelcast/client/ICountDownLatch.o ./obj/main.o ./obj/hazelcast/client/serialization/SerializationService.o ./obj/hazelcast/client/protocol/CommandHandler.o ./obj/hazelcast/client/AtomicNumber.o ./obj/hazelcast/client/serialization/PortableReader.o ./obj/hazelcast/client/serialization/DataOutput.o ./obj/hazelcast/client/protocol/Socket.o ./obj/hazelcast/client/serialization/SerializationContext.o

CFLAGS=
LFLAGS= -pthread /usr/local/lib/libz.a /usr/local/lib/libboost_thread.a /usr/local/lib/libboost_system.a -lrt

all: $(OUT)
	echo "Finished."

$(OUT): $(OBJS)
	echo "Linking $(OUT)..."
	$(CC)  $(OBJS) -o $(OUT) $(LFLAGS)

./obj/hazelcast/client/Address.o: ./cppClient/hazelcast/client/Address.cpp ./cppClient/hazelcast/client/Address.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/AtomicNumber.o: ./cppClient/hazelcast/client/AtomicNumber.cpp ./cppClient/hazelcast/client/AtomicNumber.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ClientConfig.o: ./cppClient/hazelcast/client/ClientConfig.cpp ./cppClient/hazelcast/client/Address.cpp ./cppClient/hazelcast/client/Address.h ./cppClient/hazelcast/client/ClientConfig.h ./cppClient/hazelcast/client/GroupConfig.cpp ./cppClient/hazelcast/client/GroupConfig.h ./cppClient/hazelcast/client/serialization/PortableFactory.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ClientService.o: ./cppClient/hazelcast/client/ClientService.cpp ./cppClient/hazelcast/client/ClientConfig.cpp ./cppClient/hazelcast/client/ClientConfig.h ./cppClient/hazelcast/client/ClientService.h ./cppClient/hazelcast/client/HazelcastClient.cpp ./cppClient/hazelcast/client/HazelcastClient.h ./cppClient/hazelcast/client/protocol/CommandHandler.cpp ./cppClient/hazelcast/client/protocol/CommandHandler.h ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/GroupConfig.o: ./cppClient/hazelcast/client/GroupConfig.cpp ./cppClient/hazelcast/client/GroupConfig.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/HazelcastClient.o: ./cppClient/hazelcast/client/HazelcastClient.cpp ./cppClient/hazelcast/client/AtomicNumber.cpp ./cppClient/hazelcast/client/AtomicNumber.h ./cppClient/hazelcast/client/ClientConfig.cpp ./cppClient/hazelcast/client/ClientConfig.h ./cppClient/hazelcast/client/HazelcastClient.h ./cppClient/hazelcast/client/ICountDownLatch.cpp ./cppClient/hazelcast/client/ICountDownLatch.h ./cppClient/hazelcast/client/IList.h ./cppClient/hazelcast/client/IMap.h ./cppClient/hazelcast/client/IQueue.h ./cppClient/hazelcast/client/ISemaphore.cpp ./cppClient/hazelcast/client/ISemaphore.h ./cppClient/hazelcast/client/ISet.h ./cppClient/hazelcast/client/IdGenerator.cpp ./cppClient/hazelcast/client/IdGenerator.h ./cppClient/hazelcast/client/MultiMap.h ./cppClient/hazelcast/client/protocol/CommandHandler.cpp ./cppClient/hazelcast/client/protocol/CommandHandler.h ./cppClient/hazelcast/client/protocol/GeneralCommands.h ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ICountDownLatch.o: ./cppClient/hazelcast/client/ICountDownLatch.cpp ./cppClient/hazelcast/client/ICountDownLatch.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ISemaphore.o: ./cppClient/hazelcast/client/ISemaphore.cpp ./cppClient/hazelcast/client/ISemaphore.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/IdGenerator.o: ./cppClient/hazelcast/client/IdGenerator.cpp ./cppClient/hazelcast/client/IdGenerator.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/CommandHandler.o: ./cppClient/hazelcast/client/protocol/CommandHandler.cpp ./cppClient/hazelcast/client/protocol/Command.h ./cppClient/hazelcast/client/protocol/CommandHandler.h ./cppClient/hazelcast/client/protocol/Socket.cpp ./cppClient/hazelcast/client/protocol/Socket.h ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/Socket.o: ./cppClient/hazelcast/client/protocol/Socket.cpp ./cppClient/hazelcast/client/Address.cpp ./cppClient/hazelcast/client/Address.h ./cppClient/hazelcast/client/protocol/Socket.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinition.o: ./cppClient/hazelcast/client/serialization/ClassDefinition.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.h ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/DataSerializable.h ./cppClient/hazelcast/client/serialization/FieldDefinition.cpp ./cppClient/hazelcast/client/serialization/FieldDefinition.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/Data.o: ./cppClient/hazelcast/client/serialization/Data.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.h ./cppClient/hazelcast/client/serialization/Data.h ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/DataSerializable.h ./cppClient/hazelcast/client/serialization/SerializationContext.cpp ./cppClient/hazelcast/client/serialization/SerializationContext.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataInput.o: ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.h ./cppClient/hazelcast/client/serialization/Data.cpp ./cppClient/hazelcast/client/serialization/Data.h ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/SerializationContext.cpp ./cppClient/hazelcast/client/serialization/SerializationContext.h ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataOutput.o: ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/FieldDefinition.o: ./cppClient/hazelcast/client/serialization/FieldDefinition.cpp ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/DataSerializable.h ./cppClient/hazelcast/client/serialization/FieldDefinition.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableReader.o: ./cppClient/hazelcast/client/serialization/PortableReader.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.h ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/FieldDefinition.cpp ./cppClient/hazelcast/client/serialization/FieldDefinition.h ./cppClient/hazelcast/client/serialization/Portable.h ./cppClient/hazelcast/client/serialization/PortableReader.h ./cppClient/hazelcast/client/serialization/PortableSerializer.cpp ./cppClient/hazelcast/client/serialization/PortableSerializer.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableSerializer.o: ./cppClient/hazelcast/client/serialization/PortableSerializer.cpp ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/Portable.h ./cppClient/hazelcast/client/serialization/PortableReader.cpp ./cppClient/hazelcast/client/serialization/PortableReader.h ./cppClient/hazelcast/client/serialization/PortableSerializer.h ./cppClient/hazelcast/client/serialization/PortableWriter.cpp ./cppClient/hazelcast/client/serialization/PortableWriter.h ./cppClient/hazelcast/client/serialization/SerializationConstants.h ./cppClient/hazelcast/client/serialization/SerializationContext.cpp ./cppClient/hazelcast/client/serialization/SerializationContext.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableWriter.o: ./cppClient/hazelcast/client/serialization/PortableWriter.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/FieldDefinition.cpp ./cppClient/hazelcast/client/serialization/FieldDefinition.h ./cppClient/hazelcast/client/serialization/PortableSerializer.cpp ./cppClient/hazelcast/client/serialization/PortableSerializer.h ./cppClient/hazelcast/client/serialization/PortableWriter.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationContext.o: ./cppClient/hazelcast/client/serialization/SerializationContext.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/SerializationContext.h ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationService.o: ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/ConstantSerializers.h ./cppClient/hazelcast/client/serialization/Data.cpp ./cppClient/hazelcast/client/serialization/Data.h ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h ./cppClient/hazelcast/client/serialization/DataOutput.cpp ./cppClient/hazelcast/client/serialization/DataOutput.h ./cppClient/hazelcast/client/serialization/Portable.h ./cppClient/hazelcast/client/serialization/PortableFactory.h ./cppClient/hazelcast/client/serialization/PortableSerializer.cpp ./cppClient/hazelcast/client/serialization/PortableSerializer.h ./cppClient/hazelcast/client/serialization/SerializationConstants.h ./cppClient/hazelcast/client/serialization/SerializationContext.cpp ./cppClient/hazelcast/client/serialization/SerializationContext.h ./cppClient/hazelcast/client/serialization/SerializationService.h ./cppClient/hazelcast/client/serialization/TypeSerializer.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/main.o: ./cppClient/main.cpp ./cppClient/SimpleMapTest.h ./cppClient/TestInnerPortable.h ./cppClient/TestMainPortable.h ./cppClient/TestNamedPortable.h ./cppClient/TestPortableFactory.h ./cppClient/hazelcast/client/ClientConfig.cpp ./cppClient/hazelcast/client/ClientConfig.h ./cppClient/hazelcast/client/GroupConfig.cpp ./cppClient/hazelcast/client/GroupConfig.h ./cppClient/hazelcast/client/HazelcastClient.cpp ./cppClient/hazelcast/client/HazelcastClient.h ./cppClient/hazelcast/client/IMap.h ./cppClient/hazelcast/client/serialization/Portable.h ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OUT)
	$(RM) $(OBJS)

.SILENT:

