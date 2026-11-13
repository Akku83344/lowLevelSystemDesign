/*
 * LLD Practice: Parking Lot (C++, SDE-1)
 * Nov 12, 2026 · @Akanksha Gupta
 *
 * A multi-floor lot that assigns a matching spot on entry, issues a ticket,
 * and charges by parking duration on exit. Pricing can be swapped without
 * touching the core flow.
 *
 * Assumed answers
 * - Vehicles: Bike, Car, Truck
 * - Spots: Compact, Regular, Large
 * - No bigger-spot fallback. Bike -> Compact, Car -> Regular, Truck -> Large
 * - Multiple floors, multiple gates. Every gate calls the same lot.
 * - Fee is per hour, rounded up, minimum 1 hour, rate depends on vehicle type.
 * - Spot choice is the first free matching spot, floors scanned in order.
 * - Payment is out of scope. Only the fee is calculated.
 *
 * Functional requirements
 * 1. The lot has multiple floors, and each floor has spots of different types.
 * 2. On entry, find an available spot matching the vehicle type, mark it
 *    occupied, and issue a ticket.
 * 3. If no spot is available, reject the entry.
 * 4. On exit, take the ticket, calculate the fee from duration and vehicle
 *    type, free the spot, and return the fee.
 * 5. An invalid ticket returns -1.
 * 6. Pricing is replaceable (hourly, flat rate) without changing the core flow.
 *
 * Non-functional requirements
 * - Extensible: new vehicle types, spot types, or pricing rules should not
 *   need a big rewrite.
 *
 * Entities
 * Enums
 * - VehicleType: BIKE, CAR, TRUCK
 * - SpotType: COMPACT, REGULAR, LARGE
 *
 * Classes
 * - Vehicle: plate, VehicleType. Identifies the vehicle and its type.
 * - ParkingSpot: id, SpotType, occupied flag. Free check, fit check,
 *   occupy, and free.
 * - ParkingFloor: floor number, list of spots. Finds an available spot.
 * - Ticket: id, vehicle, spot, entry time. Parking record, data only.
 * - PricingStrategy: calculateFee(ticket, exitTime).
 * - HourlyPricing, FlatRatePricing: implement PricingStrategy.
 * - ParkingLot (Singleton): floors, active tickets, pricing strategy,
 *   next ticket id. parkVehicle() and unparkVehicle().
 *
 * Relationships
 * - ParkingLot has many ParkingFloor, and each floor has many ParkingSpot.
 * - Ticket links one Vehicle to one ParkingSpot.
 * - ParkingLot uses a PricingStrategy and does not care which one.
 *
 * Design decisions
 * 1. Spot matching lives in ParkingSpot.canFit(), so ParkingLot has no
 *    if-else on vehicle type.
 * 2. Ticket is data only. Fee logic stays in the strategy.
 * 3. ParkingLot depends on the PricingStrategy interface.
 * 4. Gates are not classes. A gate calls parkVehicle() or unparkVehicle().
 *
 * Patterns
 * - Strategy on pricing (Open/Closed): a new rule is a new class.
 * - Singleton on ParkingLot: one shared state for every gate.
 *   The function-local static is initialized thread-safely in C++11+.
 *
 * UML class diagram
 *
 *                      <<Singleton>>
 *                        ParkingLot
 *        ---------------------------------------------
 *        | floors, activeTickets, pricing            |
 *        | + getInstance()                           |
 *        | + parkVehicle(vehicle) : Ticket?          |
 *        | + unparkVehicle(ticketId) : fee           |
 *        | + setPricingStrategy(strategy)            |
 *        ---------------------------------------------
 *           | 1 has            | uses           | owns
 *           | *                |                | *
 *           v                  v                v
 *     ParkingFloor      <<interface>>         Ticket
 *     --------------    PricingStrategy       --------------------
 *     | floorNumber|    ---------------       | id, entryTime    |
 *     | spots      |    | calculateFee |      | vehicle, spot    |
 *     | findSpot() |    ---------------       --------------------
 *     --------------         ^        ^          |            |
 *           | 1              |        |          |            |
 *           | *       HourlyPricing  FlatRate    |            |
 *           v                                     v            v
 *     ParkingSpot                              Vehicle    ParkingSpot
 *     -------------------                      ---------------
 *     | id, SpotType    |                      | plate, type |
 *     | occupied        |                      ---------------
 *     | canFit(vehicle) |
 *     | occupy() / free |
 *     -------------------
 *
 *     VehicleType: BIKE | CAR | TRUCK
 *     SpotType:    COMPACT | REGULAR | LARGE
 *
 * Gates are not on this diagram. They only call ParkingLot.
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ctime>
#include <optional>
#include <string>
using namespace std;

// ---------- Enums ----------
enum class VehicleType { BIKE, CAR, TRUCK };
enum class SpotType { COMPACT, REGULAR, LARGE };

// ---------- Vehicle ----------
class Vehicle {
    string plate;
    VehicleType type;
public:
    Vehicle(string p, VehicleType t) : plate(p), type(t) {}
    string getPlate() const { return plate; }
    VehicleType getType() const { return type; }
};

// ---------- ParkingSpot ----------
class ParkingSpot {
    int id;
    SpotType type;
    bool occupied = false;
public:
    ParkingSpot(int i, SpotType t) : id(i), type(t) {}
    int getId() const { return id; }
    bool isFree() const { return !occupied; }

    // Spot-matching rule lives here, not in ParkingLot
    bool canFit(const Vehicle& v) const {
        if (v.getType() == VehicleType::BIKE)  return type == SpotType::COMPACT;
        if (v.getType() == VehicleType::CAR)   return type == SpotType::REGULAR;
        if (v.getType() == VehicleType::TRUCK) return type == SpotType::LARGE;
        return false;
    }
    void occupy() { occupied = true; }
    void free() { occupied = false; }
};

// ---------- ParkingFloor ----------
class ParkingFloor {
    int floorNumber;
    vector<ParkingSpot> spots;   // floor owns its spots
public:
    ParkingFloor(int n) : floorNumber(n) {}
    int getFloorNumber() const { return floorNumber; }
    void addSpot(ParkingSpot spot) { spots.push_back(spot); }

    // returns the address of a free matching spot, or nullptr if none
    ParkingSpot* findAvailableSpot(const Vehicle& v) {
        for (auto& spot : spots)
            if (spot.isFree() && spot.canFit(v)) return &spot;
        return nullptr;
    }
};

// ---------- Ticket (data only) ----------
class Ticket {
    int id;
    Vehicle vehicle;
    ParkingSpot* spot;   // points to the spot owned by the floor
    time_t entryTime;    // seconds
public:
    Ticket(int i, Vehicle v, ParkingSpot* s, time_t t)
        : id(i), vehicle(v), spot(s), entryTime(t) {}
    int getId() const { return id; }
    Vehicle getVehicle() const { return vehicle; }
    ParkingSpot* getSpot() const { return spot; }
    time_t getEntryTime() const { return entryTime; }
};

// ---------- Strategy: pricing ----------
class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual double calculateFee(const Ticket& ticket, time_t exitTime) const = 0;
};

class HourlyPricing : public PricingStrategy {
public:
    double calculateFee(const Ticket& ticket, time_t exitTime) const override {
        long minutes = (exitTime - ticket.getEntryTime()) / 60;
        long hours = (minutes + 59) / 60;   // round up to the next hour
        if (hours < 1) hours = 1;           // minimum 1 hour

        VehicleType t = ticket.getVehicle().getType();
        if (t == VehicleType::BIKE) return hours * 10;
        if (t == VehicleType::CAR)  return hours * 20;
        return hours * 30;
    }
};

class FlatRatePricing : public PricingStrategy {
public:
    double calculateFee(const Ticket& ticket, time_t) const override {
        VehicleType t = ticket.getVehicle().getType();
        if (t == VehicleType::BIKE) return 50;
        if (t == VehicleType::CAR)  return 100;
        return 150;
    }
};

// ---------- Singleton: ParkingLot ----------
class ParkingLot {
    vector<ParkingFloor> floors;
    unordered_map<int, Ticket> activeTickets;   // ticket id -> Ticket
    unique_ptr<PricingStrategy> pricing;
    int nextTicketId = 1;

    ParkingLot() : pricing(make_unique<HourlyPricing>()) {}   // private constructor

public:
    ParkingLot(const ParkingLot&) = delete;
    ParkingLot& operator=(const ParkingLot&) = delete;

    static ParkingLot& getInstance() {
        static ParkingLot instance;   // thread-safe init in C++11+
        return instance;
    }

    void addFloor(ParkingFloor floor) {
        floors.push_back(floor);
    }

    void setPricingStrategy(unique_ptr<PricingStrategy> strategy) {
        // unique_ptr cannot be copied (only one owner), so we move ownership:
        // 'pricing' takes the new strategy, 'strategy' becomes empty,
        // and the old strategy is deleted automatically.
        pricing = move(strategy);
    }

    optional<Ticket> parkVehicle(const Vehicle& vehicle, time_t entryTime = time(nullptr)) {
        for (auto& floor : floors) {
            ParkingSpot* spot = floor.findAvailableSpot(vehicle);
            if (spot != nullptr) {
                spot->occupy();
                Ticket ticket(nextTicketId++, vehicle, spot, entryTime);
                activeTickets.emplace(ticket.getId(), ticket);
                return ticket;
            }
        }
        return nullopt;   // no spot available
    }

    // returns the fee, or -1 if the ticket is invalid
    double unparkVehicle(int ticketId, time_t exitTime = time(nullptr)) {
        auto it = activeTickets.find(ticketId);
        if (it == activeTickets.end()) return -1;

        double fee = pricing->calculateFee(it->second, exitTime);
        it->second.getSpot()->free();
        activeTickets.erase(it);
        return fee;
    }
};

// ---------- Demo ----------
int main() {
    ParkingLot& lot = ParkingLot::getInstance();

    ParkingFloor f1(1);
    f1.addSpot(ParkingSpot(1, SpotType::COMPACT));
    f1.addSpot(ParkingSpot(2, SpotType::REGULAR));
    ParkingFloor f2(2);
    f2.addSpot(ParkingSpot(3, SpotType::REGULAR));
    f2.addSpot(ParkingSpot(4, SpotType::LARGE));
    lot.addFloor(f1);
    lot.addFloor(f2);

    Vehicle car1("KA01-1111", VehicleType::CAR);
    Vehicle car2("KA01-2222", VehicleType::CAR);
    Vehicle car3("KA01-3333", VehicleType::CAR);
    Vehicle bike("KA01-4444", VehicleType::BIKE);

    auto t1 = lot.parkVehicle(car1);
    auto t2 = lot.parkVehicle(car2);
    auto t3 = lot.parkVehicle(car3);   // no regular spot left
    auto t4 = lot.parkVehicle(bike);

    cout << "car1 ticket: " << (t1 ? to_string(t1->getId()) : "none") << "\n";
    cout << "car2 ticket: " << (t2 ? to_string(t2->getId()) : "none") << "\n";
    cout << "car3 ticket: " << (t3 ? to_string(t3->getId()) : "none (lot full for cars)") << "\n";
    cout << "bike ticket: " << (t4 ? to_string(t4->getId()) : "none") << "\n";

    time_t exitTime = time(nullptr) + 190 * 60;   // 3h 10m later
    cout << "car1 fee (hourly): " << lot.unparkVehicle(t1->getId(), exitTime) << "\n";

    lot.setPricingStrategy(make_unique<FlatRatePricing>());
    cout << "bike fee (flat):   " << lot.unparkVehicle(t4->getId(), exitTime) << "\n";

    auto t5 = lot.parkVehicle(car3);   // car1's spot is free again
    cout << "car3 retry ticket: " << (t5 ? to_string(t5->getId()) : "none") << "\n";
    cout << "invalid ticket fee: " << lot.unparkVehicle(999) << "\n";
    return 0;
}
