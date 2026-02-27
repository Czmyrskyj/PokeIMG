#!/bin/bash

echo -e "\033[94mDetecting package manager...\033[0m"
if command -v dnf &> /dev/null; then
    sudo dnf install -y boost-devel openssl-devel clang
elif command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y libboost-all-dev libssl-dev clang curl
else
    echo -e "\033[91mUnsupported package manager. Please install Boost and OpenSSL manually.\033[0m"
    exit 1
fi
if [ ! -f /usr/local/include/stb_image.h ]; then
    echo -e "\033[94mDownloading stb_image.h...\033[0m"
    sudo curl -L https://raw.githubusercontent.com/nothings/stb/master/stb_image.h -o /usr/local/include/stb_image.h
fi
echo -e "\033[94mCompiling Pokeimg...\033[0m"
if clang++ -std=c++23 -O3 main.cpp -o pokeimg -lboost_system -lssl -lcrypto -lpthread -lboost_json; then
    echo -e "\033[94mInstalling to /usr/local/bin...\033[0m"
    sudo mv pokeimg /usr/local/bin/
    sudo chmod +x /usr/local/bin/pokeimg
    echo -e "\033[92mSuccess! Type: pokeimg --help for more info.\033[0m"
else
    echo -e "\033[91mCompilation failed!\033[0m"
    exit 1
fi
