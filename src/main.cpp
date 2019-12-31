#include <iostream>
#include <core/Application.h>

int main()
{
    try {
        vmc::Application app(800, 600);
        app.run();
    }
    catch (std::runtime_error e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
    return 0;
}