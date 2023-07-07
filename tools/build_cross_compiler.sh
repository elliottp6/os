# this script sets up a x86_64-elf cross compiler
# for more reading, see https://wiki.osdev.org/GCC_Cross-Compiler

# install dependencies, along w/ dumpelf/readelf tools (from pax-utils) (note: libcloog-isl-dev omitted since it's missing & optional)
sudo apt install pax-utils build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev

# setup environment variables & script constants
export PREFIX="$HOME/opt/cross64"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"
readonly GCC=gcc-13.1.0
readonly BINUTILS=binutils-2.40
readonly SOURCE=src64

# download & extract binutils & gcc into ~/src
mkdir -p ~/${SOURCE}
cd ~/${SOURCE}
wget --no-clobber https://ftp.gnu.org/gnu/binutils/${BINUTILS}.tar.xz && tar -xf ${BINUTILS}.tar.xz
wget --no-clobber https://ftp.gnu.org/gnu/gcc/${GCC}/${GCC}.tar.xz && tar -xf ${GCC}.tar.xz

# make build-binutils
mkdir -p build-binutils
cd build-binutils
../${BINUTILS}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

# make gcc
cd ~/${SOURCE}
mkdir -p build-gcc
cd build-gcc
../${GCC}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# test gcc
$HOME/opt/cross/bin/$TARGET-gcc --version
