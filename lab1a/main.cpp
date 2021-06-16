#include "logging.h"
#include "Car.h"
#include "random.h"
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
    int n = stoi(argv[1]);
    vector<Car> cars;
    cars.reserve(n);

    for (int i = 0; i < n; i++) {
        Car car{};
        car.direction = randomInt(0, 1);
        car.registration = randomInt(1000, 9999);
        cars.push_back(car);
    }

    // compile first
    log("Compiling...");
    system("g++ ../Auto.cpp -o ../Auto.out && g++ ../Semafor.cpp -o ../Semafor.out");
    log("Done.");

    for (int i = 0; i < n; i++) {
        string command = "../Auto.out " + to_string(cars[i].registration) + " " + to_string(cars[i].direction) + " &";
        system(command.c_str());
    }

    string command = "../Semafor.out " + to_string(n);
    system(command.c_str());
    return 0;
}
