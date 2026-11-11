/*
 * LLD Design Patterns
 * Oct 29, 2026 · @Akanksha Gupta
 *
 * 5. State Pattern
 *
 * Definition
 * The State Pattern allows an object to alter its behavior when its internal
 * state changes. This pattern encapsulates state-specific behaviors into
 * separate classes, promoting cleaner code and better organization.
 *
 * State Transition Diagram
 *
 *  +----------+             +-----------+             +-------------+
 *  |  Placed  | ----------> |  Shipped  | ----------> |  Delivered  |
 *  +----------+  proceed()  +-----------+  proceed()  +-------------+
 *                                                      ^       |
 *                                                      +-------+
 *                                                  proceed(): no change
 *
 * Order (the context) holds the current state and forwards proceed() to it.
 * Each concrete state decides the next state:
 * - Placed  -> Shipped
 * - Shipped -> Delivered
 * - Delivered stays Delivered
 *
 * The handleProceed definitions sit below Order because they call setState
 * and the state accessors, which are not known until Order is fully declared.
 */

#include <iostream>
#include <string>
using namespace std;

class Order;

class OrderState {
public:
    virtual ~OrderState() = default;
    virtual void handleProceed(Order& o) = 0;
    virtual string name() = 0;
};

class PlacedState : public OrderState {
public:
    void handleProceed(Order& o) override;
    string name() override { return "Placed"; }
};

class ShippedState : public OrderState {
public:
    void handleProceed(Order& o) override;
    string name() override { return "Shipped"; }
};

class DeliveredState : public OrderState {
public:
    void handleProceed(Order& o) override;
    string name() override { return "Delivered"; }
};

class Order {
    PlacedState placed;
    ShippedState shipped;
    DeliveredState delivered;
    OrderState* current;

public:
    Order() : current(&placed) {}

    void setState(OrderState* s) { current = s; }
    OrderState* getShipped()     { return &shipped; }
    OrderState* getDelivered()   { return &delivered; }

    void proceed() { current->handleProceed(*this); }
    void show()    { cout << "Order status: " << current->name() << "\n"; }
};

void PlacedState::handleProceed(Order& o) {
    cout << "Packing and shipping...\n";
    o.setState(o.getShipped());
}

void ShippedState::handleProceed(Order& o) {
    cout << "Delivering...\n";
    o.setState(o.getDelivered());
}

void DeliveredState::handleProceed(Order& o) {
    cout << "Already delivered, nothing to do\n";
}

int main() {
    Order order;

    order.show();
    order.proceed();
    order.show();
    order.proceed();
    order.show();
    order.proceed();
}
