1. cd into the directory
2. make
3. make producer && make consumer
4. insmod numpipe N=50
5. start producer: ./producer /dev/numpipe
6. start consumer: ./consumer /dev/numpipe
7. stop: control+c

![Image of Yaktocat]
(https://imgur.com/jwkauFE)
