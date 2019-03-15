#include "queue.hpp"

#include <iostream>

int main(){

    queue<int> myints(-1, true);

    myints.write(1);
    myints.write(2);
    int ret = myints.write_nb(3);

    std::cout << "Size " << ret << "\n";

    std::cout << "Read " << myints.read() << "\n";
    std::cout << "Read " << myints.read() << "\n";
    myints.write(3);
    std::cout << "Read " << myints.read() << "\n";

}
