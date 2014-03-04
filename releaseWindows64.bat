mkdir .\cpp\Windows_64\hazelcast\lib
mkdir .\cpp\Windows_64\hazelcast\include\hazelcast\
mkdir .\cpp\Windows_64\external\lib
mkdir .\cpp\Windows_64\external\include

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\

copy build64\Release\HazelcastClientShared.exp  cpp\Windows_64\hazelcast\lib\HazelcastClientShared_64.exp
copy build64\Release\HazelcastClientShared.pdb  cpp\Windows_64\hazelcast\lib\HazelcastClientShared_64.pdb
copy build64\Release\HazelcastClientShared.dll  cpp\Windows_64\hazelcast\lib\HazelcastClientShared_64.dll
copy build64\Release\HazelcastClientShared.lib  cpp\Windows_64\hazelcast\lib\HazelcastClientShared_64.lib
copy build64\Release\HazelcastClientStatic.lib  cpp\Windows_64\hazelcast\lib\HazelcastClientStatic_64.lib

xcopy /S /Q external\include\* cpp\Windows_64\external\include\
xcopy /S /Q external\lib\windows\64\* cpp\Windows_64\external\lib\