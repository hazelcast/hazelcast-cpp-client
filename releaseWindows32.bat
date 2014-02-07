mkdir .\releaseBundle32\hazelcast\lib
mkdir .\releaseBundle32\hazelcast\include\hazelcast\
mkdir .\releaseBundle32\external\lib
mkdir .\releaseBundle32\external\include
mkdir .\releaseBundle32\docs\

copy build32\Release\HazelcastClientShared.exp  releaseBundle32\hazelcast\lib\HazelcastClientShared_32.exp
copy build32\Release\HazelcastClientShared.pdb  releaseBundle32\hazelcast\lib\HazelcastClientShared_32.pdb
copy build32\Release\HazelcastClientShared.dll  releaseBundle32\hazelcast\lib\HazelcastClientShared_32.dll
copy build32\Release\HazelcastClientShared.lib  releaseBundle32\hazelcast\lib\HazelcastClientShared_32.lib
copy build32\Release\HazelcastClientStatic.lib  releaseBundle32\hazelcast\lib\HazelcastClientStatic_32.lib
copy enterprise-license.txt releaseBundle32\enterprise-license.txt

xcopy /S /Q docs\* releaseBundle32\docs\
xcopy /S /Q external\include\* releaseBundle32\external\include\
xcopy /S /Q hazelcast\include\hazelcast\* releaseBundle32\hazelcast\include\hazelcast\
xcopy /S /Q external\lib\windows\32\* releaseBundle32\external\lib\