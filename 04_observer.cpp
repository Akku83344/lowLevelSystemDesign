/*
 * LLD Design Patterns
 * Oct 29, 2026 · @Akanksha Gupta
 *
 * 4. Observer Pattern
 *
 * Definition
 * Observer is a behavioral pattern where a subject keeps a list of subscribers
 * and automatically notifies all of them whenever its state changes, without
 * knowing who they are or what they do with the update.
 *
 * Real-life picture
 * A YouTube channel (Subject) and its subscribers (Observers). When a new
 * video is uploaded, every subscriber gets a notification. The channel doesn't
 * know or care what each subscriber does with it.
 *
 * Where it's used
 * - Notifications and events
 * - Parking lot: display boards updating when a slot becomes free
 * - Stock price apps, UI button clicks, chat apps
 *
 * In this example the Subject is Order. EmailNotifier, SmsNotifier, and
 * PushNotifier are concrete observers. setStatus changes state and calls
 * notifyAll(), which tells every current subscriber. unsubscribe removes one
 * subscriber so later updates skip it.
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// 1. OBSERVER INTERFACE
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const string& status) = 0;
};

// 2. CONCRETE OBSERVERS
class EmailNotifier : public Observer {
public:
    void update(const string& status) override {
        cout << "[Email] Your order is " << status << "\n";
    }
};

class SmsNotifier : public Observer {
public:
    void update(const string& status) override {
        cout << "[SMS] Order update: " << status << "\n";
    }
};

class PushNotifier : public Observer {
public:
    void update(const string& status) override {
        cout << "[Push] Order " << status << "\n";
    }
};

// 3. SUBJECT
class Order {
    vector<Observer*> observers;
    string status = "placed";
public:
    void subscribe(Observer* o) {
        observers.push_back(o);
    }

    void unsubscribe(Observer* o) {
        observers.erase(remove(observers.begin(), observers.end(), o), observers.end());
    }

    void setStatus(const string& s) {
        status = s;
        notifyAll();                  // state changed, so tell everyone
    }

private:
    void notifyAll() {
        for (Observer* o : observers)
            o->update(status);
    }
};

int main() {
    Order order;
    EmailNotifier email;
    SmsNotifier sms;
    PushNotifier push;

    order.subscribe(&email);
    order.subscribe(&sms);
    order.subscribe(&push);

    order.setStatus("shipped");       // all three notified

    order.unsubscribe(&sms);          // customer turns off SMS
    order.setStatus("delivered");     // only email and push notified
}
