# Installs Boost from source
# usage: .\install-boost.ps1 <version>

Set-Variable version $args[0]
Set-Variable tarball_name ("boost_" + $version.Replace('.', '_'))

Invoke-WebRequest `
    "https://dl.bintray.com/boostorg/release/$version/source/$tarball_name.tar.gz" `
    -OutFile "$tarball_name.tar.gz"

tar xzf "$tarball_name.tar.gz"
cd $tarball_name
.\bootstrap.bat
.\b2 --with-thread --with-chrono install
