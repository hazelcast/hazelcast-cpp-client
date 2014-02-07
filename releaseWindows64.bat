mkdir .\releaseBundle64\hazelcast\lib
mkdir .\releaseBundle64\hazelcast\include\hazelcast\
mkdir .\releaseBundle64\external\lib
mkdir .\releaseBundle64\external\include
mkdir .\releaseBundle64\docs\

copy build64\Release\HazelcastClientShared.exp  releaseBundle64\hazelcast\lib\HazelcastClientShared_64.exp
copy build64\Release\HazelcastClientShared.pdb  releaseBundle64\hazelcast\lib\HazelcastClientShared_64.pdb
copy build64\Release\HazelcastClientShared.dll  releaseBundle64\hazelcast\lib\HazelcastClientShared_64.dll
copy build64\Release\HazelcastClientShared.lib  releaseBundle64\hazelcast\lib\HazelcastClientShared_64.lib
copy build64\Release\HazelcastClientStatic.lib  releaseBundle64\hazelcast\lib\HazelcastClientStatic_64.lib
copy enterprise-license.txt releaseBundle64\enterprise-license.txt

xcopy /S /Q docs\* releaseBundle64\docs\
xcopy /S /Q external\include\* releaseBundle64\external\include\
xcopy /S /Q hazelcast\include\hazelcast\* releaseBundle64\hazelcast\include\hazelcast\
xcopy /S /Q external\lib\windows\64\* releaseBundle64\external\lib\