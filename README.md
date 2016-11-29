This is VOTCA's next generation build system. 

Usage:
```
git clone --recursive https://github.com/votca/votca.git
cd votca
mkdir build
cd build
cmake -DBUILD_CSGAPPS=ON -DBUILD_CTP=ON -DBUILD_XTP=ON ..
make -j5
```

Further information on VOTCA can be found at http://www.votca.org
