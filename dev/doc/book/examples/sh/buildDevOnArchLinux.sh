sudo pacman -S git gcc make
git clone https://github.com/octaspire/dern.git
cd dern
make submodules-init
make
