curl -O octaspire.com/dern.tar.bz2
curl -O https://octaspire.io/dern.sha512
sha512sum -c dern.sha512
tar jxf dern.tar.bz2
cd dern
sh how-to-build/YOUR_PLATFORM_NAME_HERE.XX
