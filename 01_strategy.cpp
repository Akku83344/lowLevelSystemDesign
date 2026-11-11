/*
 * LLD Design Patterns
 * Oct 29, 2026 · @Akanksha Gupta
 *
 * 1. Strategy Pattern
 * Type: Behavioral Design Pattern
 *
 * Definition
 * The Strategy Pattern defines a family of algorithms, encapsulates each one
 * behind a common interface, and makes them interchangeable. It allows the
 * algorithm or behavior to be selected or changed at runtime without changing
 * the code that uses it.
 *
 * Components
 * 1. Strategy Interface: Defines a common interface for all supported algorithms.
 * 2. Concrete Strategies: Implement the strategy interface with different behaviors.
 * 3. Context: Uses a strategy through the common interface and can switch
 *    between strategies.
 *
 * Class Relationship
 *
 *              PaymentStrategy
 *              (Interface)
 *                    ^
 *                    |
 *           +--------+--------+
 *           |                 |
 *      CardPayment        UpiPayment
 *           ^
 *           |
 *        Checkout
 *   (uses PaymentStrategy)
 *
 * Checkout uses the PaymentStrategy interface. CardPayment and UpiPayment
 * provide different implementations of the same interface.
 *
 * Pure virtual (= 0): makes PaymentStrategy an abstract class. It cannot be
 * instantiated, and every derived class must implement pay().
 *
 * Virtual destructor: a base class meant for inheritance needs one, so delete
 * through a base pointer also runs the derived destructor. Without it,
 * behavior is undefined and derived resources leak.
 */

#include <iostream>
#include <string>
using namespace std;

class PaymentStrategy {
public:
    virtual void pay(double amount) = 0;
    virtual ~PaymentStrategy() = default;
};

class CardPayment : public PaymentStrategy {
    string cardNumber;

public:
    explicit CardPayment(string card) : cardNumber(card) {}

    void pay(double amount) override {
        cout << "Paid " << amount
             << " using Card: " << cardNumber << "\n";
    }
};

class UpiPayment : public PaymentStrategy {
    string upiId;

public:
    explicit UpiPayment(string upi) : upiId(upi) {}

    void pay(double amount) override {
        cout << "Paid " << amount
             << " using UPI: " << upiId << "\n";
    }
};

class Checkout {
    PaymentStrategy* strategy;

public:
    explicit Checkout(PaymentStrategy* s) : strategy(s) {}

    void setStrategy(PaymentStrategy* s) {
        strategy = s;
    }

    void payAmount(double amount) {
        strategy->pay(amount);
    }
};

int main() {
    CardPayment card("1234");
    UpiPayment upi("user@upi");

    Checkout checkout(&card);
    checkout.payAmount(500);

    checkout.setStrategy(&upi);
    checkout.payAmount(1000);
}
