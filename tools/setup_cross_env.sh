# note: this file must be executed via "source ./setup_cross_env.sh
# that way, the exports appear in the calling environment (otherwise, they'll be script-local)
export PREFIX="$HOME/opt/cross64"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

