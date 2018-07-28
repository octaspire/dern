su root
pkgin install clang binutils git-base
exit
git clone git://github.com/octaspire/dern
cd dern
perl -pi -e 's/https/git/' .gitmodules
make submodules-init
make
