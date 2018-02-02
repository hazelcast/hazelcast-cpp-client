We use embedded python code to utilize the python hazelcast-remote-controller client. It is used to start and stop members and create stop clusters. In order to make for the embedded python to work, the requirements are:

- The python hazelcast-remote-controller library is installed using command pip install -r hazelcast/test/test_requirements.txt
- Windows: At windows the python comes only with release 32-bit libraries. But we need debug libraries and 64-bit versions. Therefore, we had to compile the python at Windows and point to this directory while linking and running the test executable. The libraries are:
    - For 32-bit: set PYTHON_LIB_DIR=C:\Python-2.7.14\PCbuild 
    - For 64-bit: set PYTHON_LIB_DIR=C:\Python-2.7.14\PCbuild\amd64
    
    Then we have to include and link to python in this way when running cmake:
    - -DPYTHON_INCLUDE_DIR=C:\Python27\include -DPYTHON_LIBRARY=%PYTHON_LIBRARY_PATH%
    
    Furthermore, when running the test application we have to set these environment variables:
    set PYTHONHOME=C:\Python27
    set PYTHONPATH=%PYTHON_LIB_DIR%
    SET PATH=%PYTHON_LIB_DIR%;%PATH%

    - How do we compile python at windows, run command:
        - 32-bit Debug: PCbuild\build.bat "/p:Flavor=Debug;Configuration=Debug;VisualStudioVersion=12.0;Platform=win32;PlatformTarget=win32;PlatformToolset=v120"
        - 32-bit Release: PCbuild\build.bat "/p:Flavor=Release;Configuration=Release;VisualStudioVersion=12.0;Platform=win32;PlatformTarget=win32;PlatformToolset=v120"
        - 64-bit Debug: PCbuild\build.bat "/p:Flavor=Debug;Configuration=Debug;VisualStudioVersion=12.0;Platform=x64;PlatformTarget=win32;PlatformToolset=v120"
        - 64-bit Release: PCbuild\build.bat "/p:Flavor=Release;Configuration=Release;VisualStudioVersion=12.0;Platform=x64;PlatformTarget=win32;PlatformToolset=v120"
        