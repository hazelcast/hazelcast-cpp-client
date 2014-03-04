mkdir .\cpp\Windows_32\hazelcast\lib
mkdir .\cpp\Windows_32\hazelcast\include\hazelcast\
mkdir .\cpp\Windows_32\external\lib
mkdir .\cpp\Windows_32\external\include

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\

copy build32\Release\HazelcastClientShared.exp  cpp\Windows_32\hazelcast\lib\HazelcastClientShared_32.exp
copy build32\Release\HazelcastClientShared.pdb  cpp\Windows_32\hazelcast\lib\HazelcastClientShared_32.pdb
copy build32\Release\HazelcastClientShared.dll  cpp\Windows_32\hazelcast\lib\HazelcastClientShared_32.dll
copy build32\Release\HazelcastClientShared.lib  cpp\Windows_32\hazelcast\lib\HazelcastClientShared_32.lib
copy build32\Release\HazelcastClientStatic.lib  cpp\Windows_32\hazelcast\lib\HazelcastClientStatic_32.lib

xcopy /S /Q external\include\* cpp\Windows_32\external\include\
xcopy /S /Q external\lib\windows\32\* cpp\Windows_32\external\lib\