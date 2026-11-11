/*
 * LLD Design Patterns
 * Oct 29, 2026 · @Akanksha Gupta
 *
 * 2. Factory Method Pattern
 * Type: Creational Design Pattern
 *
 * Definition
 * The Factory Method Pattern defines a method for creating objects, while
 * allowing subclasses to decide which concrete class to instantiate. The code
 * using the created object can work through a common interface without
 * depending directly on concrete classes.
 *
 * Components
 * 1. Product Interface: Defines the common interface for the objects being created.
 * 2. Concrete Products: Implement the product interface.
 * 3. Creator: Declares the factory method and can contain operations that use
 *    the created product.
 * 4. Concrete Creators: Implement the factory method to decide which concrete
 *    product to create.
 *
 * Class Relationship
 *
 *                   Vehicle
 *               (Product Interface)
 *                      ^
 *                      |
 *              +-------+-------+
 *              |               |
 *             Car            Truck
 *
 *
 *                VehicleCreator
 *                   (Creator)
 *                      ^
 *                      |
 *              +-------+-------+
 *              |               |
 *          CarCreator      TruckCreator
 *              |               |
 *           creates           creates
 *              |               |
 *             Car             Truck
 *
 * This file contains both examples from the notes:
 * - simple_factory: one class picks the product from a type string.
 *   static belongs to the class, not to an object. Call it as
 *   VehicleFactory::create() without creating a VehicleFactory.
 *   unique_ptr: single owner, and the object is deleted automatically when
 *   the pointer goes out of scope, so no manual delete. make_unique creates it.
 * - factory_method: each creator subclass decides which product to build.
 *
 * The two examples use the same class names, so each lives in its own namespace.
 */

#include <iostream>
#include <memory>
#include <string>
using namespace std;

namespace simple_factory {

class Vehicle {
public:
    virtual ~Vehicle() = default;
    virtual void drive() = 0;
};

class Car : public Vehicle {
public:
    void drive() override { cout << "Driving a car\n"; }
};

class Truck : public Vehicle {
public:
    void drive() override { cout << "Driving a truck\n"; }
};

class VehicleFactory {
public:
    static unique_ptr<Vehicle> create(const string& type) {
        if (type == "car")   return make_unique<Car>();
        if (type == "truck") return make_unique<Truck>();
        return nullptr;
    }
};

void run() {
    unique_ptr<Vehicle> v = VehicleFactory::create("truck");
    if (v) v->drive();      // Driving a truck
}

}  // namespace simple_factory

namespace factory_method {

class Vehicle {
public:
    virtual ~Vehicle() = default;
    virtual void drive() = 0;
};

class Car : public Vehicle {
public:
    void drive() override { cout << "Driving a car\n"; }
};

class Truck : public Vehicle {
public:
    void drive() override { cout << "Driving a truck\n"; }
};

class VehicleCreator {
public:
    virtual ~VehicleCreator() = default;
    virtual unique_ptr<Vehicle> create() = 0;   // children decide what to build

    void deliver() {
        unique_ptr<Vehicle> v = create();
        v->drive();
    }
};

class CarCreator : public VehicleCreator {
public:
    unique_ptr<Vehicle> create() override { return make_unique<Car>(); }
};

class TruckCreator : public VehicleCreator {
public:
    unique_ptr<Vehicle> create() override { return make_unique<Truck>(); }
};

void run() {
    TruckCreator t;
    t.deliver();    // Driving a truck

    CarCreator c;
    c.deliver();    // Driving a car
}

}  // namespace factory_method

int main() {
    simple_factory::run();
    factory_method::run();
}
