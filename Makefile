CC=clang++
OUT=p.out
RM=rm -f

OBJS=./obj/hazelcast/client/serialization/DefaultPortableWriter.o ./obj/hazelcast/client/ClientConfig.o ./obj/hazelcast/client/serialization/MorphingPortableReader.o ./obj/hazelcast/client/serialization/DataInput.o ./obj/hazelcast/client/serialization/Data.o ./obj/hazelcast/client/serialization/FieldDefinition.o ./obj/hazelcast/client/GroupConfig.o ./obj/hazelcast/client/IMap.o ./obj/hazelcast/client/serialization/ClassDefinition.o  ./obj/hazelcast/client/Address.o  ./obj/hazelcast/client/serialization/ClassDefinitionWriter.o ./obj/hazelcast/client/serialization/PortableSerializer.o ./obj/hazelcast/client/HazelcastClient.o ./obj/main.o ./obj/hazelcast/client/serialization/SerializationService.o  ./obj/hazelcast/client/protocol/CommandHandler.o ./obj/hazelcast/client/serialization/PortableReader.o ./obj/hazelcast/client/serialization/DataOutput.o ./obj/hazelcast/client/protocol/Socket.o ./obj/hazelcast/client/serialization/SerializationContext.o 

CFLAGS= -std=c++11 -stdlib=libc++
LFLAGS= -std=c++11 -stdlib=libc++ /usr/local/lib/libz.a

all: $(OUT)
	echo "Finished."

$(OUT): $(OBJS)
	echo "Linking $(OUT)..."
	$(CC) $(LFLAGS) $(OBJS) -o $(OUT)

./obj/hazelcast/client/Address.o: ./cppClient/hazelcast/client/Address.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ClientConfig.o: ./cppClient/hazelcast/client/ClientConfig.cpp ./cppClient/hazelcast/client/ClientConfig.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/GroupConfig.o: ./cppClient/hazelcast/client/GroupConfig.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/HazelcastClient.o: ./cppClient/hazelcast/client/HazelcastClient.cpp ./cppClient/hazelcast/client/HazelcastClient.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/IMap.o: ./cppClient/hazelcast/client/IMap.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@


./obj/hazelcast/client/protocol/CommandHandler.o: ./cppClient/hazelcast/client/protocol/CommandHandler.cpp ./cppClient/hazelcast/client/protocol/CommandHandler.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/Socket.o: ./cppClient/hazelcast/client/protocol/Socket.cpp ./cppClient/hazelcast/client/protocol/Socket.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinition.o: ./cppClient/hazelcast/client/serialization/ClassDefinition.cpp ./cppClient/hazelcast/client/serialization/ClassDefinition.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinitionWriter.o: ./cppClient/hazelcast/client/serialization/ClassDefinitionWriter.cpp ./cppClient/hazelcast/client/serialization/ClassDefinitionWriter.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/Data.o: ./cppClient/hazelcast/client/serialization/Data.cpp ./cppClient/hazelcast/client/serialization/Data.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataInput.o: ./cppClient/hazelcast/client/serialization/DataInput.cpp ./cppClient/hazelcast/client/serialization/DataInput.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataOutput.o: ./cppClient/hazelcast/client/serialization/DataOutput.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DefaultPortableWriter.o: ./cppClient/hazelcast/client/serialization/DefaultPortableWriter.cpp ./cppClient/hazelcast/client/serialization/DefaultPortableWriter.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/FieldDefinition.o: ./cppClient/hazelcast/client/serialization/FieldDefinition.cpp ./cppClient/hazelcast/client/serialization/FieldDefinition.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/MorphingPortableReader.o: ./cppClient/hazelcast/client/serialization/MorphingPortableReader.cpp ./cppClient/hazelcast/client/serialization/MorphingPortableReader.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableReader.o: ./cppClient/hazelcast/client/serialization/PortableReader.cpp ./cppClient/hazelcast/client/serialization/PortableReader.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableSerializer.o: ./cppClient/hazelcast/client/serialization/PortableSerializer.cpp ./cppClient/hazelcast/client/serialization/PortableSerializer.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationContext.o: ./cppClient/hazelcast/client/serialization/SerializationContext.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationService.o: ./cppClient/hazelcast/client/serialization/SerializationService.cpp ./cppClient/hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/main.o: ./cppClient/main.cpp ./cppClient/TestInnerPortable.h ./cppClient/TestMainPortable.h ./cppClient/TestNamedPortable.h ./cppClient/TestPortableFactory.h ./cppClient/hazelcast/client/serialization/Portable.h  ./obj/hazelcast/client/ClientConfig.o ./obj/hazelcast/client/GroupConfig.o ./obj/hazelcast/client/HazelcastClient.o ./obj/hazelcast/client/IMap.o ./obj/hazelcast/client/serialization/Data.o ./obj/hazelcast/client/serialization/SerializationService.o
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OUT)
	$(RM) $(OBJS)

.SILENT:

