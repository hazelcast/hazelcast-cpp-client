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

./obj/hazelcast/client/Address.o: ./hazelcast/client/Address.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/ClientConfig.o: ./hazelcast/client/ClientConfig.cpp ./hazelcast/client/ClientConfig.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/GroupConfig.o: ./hazelcast/client/GroupConfig.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/HazelcastClient.o: ./hazelcast/client/HazelcastClient.cpp ./hazelcast/client/HazelcastClient.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/IMap.o: ./hazelcast/client/IMap.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@


./obj/hazelcast/client/protocol/CommandHandler.o: ./hazelcast/client/protocol/CommandHandler.cpp ./hazelcast/client/protocol/CommandHandler.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/protocol/Socket.o: ./hazelcast/client/protocol/Socket.cpp ./hazelcast/client/protocol/Socket.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinition.o: ./hazelcast/client/serialization/ClassDefinition.cpp ./hazelcast/client/serialization/ClassDefinition.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/ClassDefinitionWriter.o: ./hazelcast/client/serialization/ClassDefinitionWriter.cpp ./hazelcast/client/serialization/ClassDefinitionWriter.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/Data.o: ./hazelcast/client/serialization/Data.cpp ./hazelcast/client/serialization/Data.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataInput.o: ./hazelcast/client/serialization/DataInput.cpp ./hazelcast/client/serialization/DataInput.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DataOutput.o: ./hazelcast/client/serialization/DataOutput.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/DefaultPortableWriter.o: ./hazelcast/client/serialization/DefaultPortableWriter.cpp ./hazelcast/client/serialization/DefaultPortableWriter.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/FieldDefinition.o: ./hazelcast/client/serialization/FieldDefinition.cpp ./hazelcast/client/serialization/FieldDefinition.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/MorphingPortableReader.o: ./hazelcast/client/serialization/MorphingPortableReader.cpp ./hazelcast/client/serialization/MorphingPortableReader.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableReader.o: ./hazelcast/client/serialization/PortableReader.cpp ./hazelcast/client/serialization/PortableReader.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/PortableSerializer.o: ./hazelcast/client/serialization/PortableSerializer.cpp ./hazelcast/client/serialization/PortableSerializer.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationContext.o: ./hazelcast/client/serialization/SerializationContext.cpp
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/hazelcast/client/serialization/SerializationService.o: ./hazelcast/client/serialization/SerializationService.cpp ./hazelcast/client/serialization/SerializationService.h
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

./obj/main.o: ./main.cpp ./TestInnerPortable.h ./TestMainPortable.h ./TestNamedPortable.h ./TestPortableFactory.h ./hazelcast/client/serialization/Portable.h  ./obj/hazelcast/client/ClientConfig.o ./obj/hazelcast/client/GroupConfig.o ./obj/hazelcast/client/HazelcastClient.o ./obj/hazelcast/client/IMap.o ./obj/hazelcast/client/serialization/Data.o ./obj/hazelcast/client/serialization/SerializationService.o
	echo "Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OUT)
	$(RM) $(OBJS)

.SILENT:

