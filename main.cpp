// main.cpp - Mo phong NGA TU 4 HUONG (Bac, Nam, Dong, Tay).
//
// Chuong trinh nay CHI dung nhung gi nhom da co (Road, Lane, Intersection, TrafficLight, Vehicle, Simulation),
// khong sua file nao khac. Moi huong gom:
//   - 1 Road "vao" (xe chay tu dau duong toi vach dung o cuoi duong, nam o position == length)
//   - 1 Road "ra"  (xe di tiep sau khi qua den, toi cuoi duong thi roi khoi he thong)
//
// Ba nhom doi tuong va ai so huu ai (xem phan giai thich Composite):
//   Simulation --so huu--> Road --so huu--> Lane --so huu--> Vehicle
//   Simulation --so huu--> Intersection --so huu--> TrafficLight
// Cac con tro Lane* / Intersection* trong Scenario chi de NHIN, khong phai chu so huu.
 
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
 
#include "Simulation/Simulation.h"
#include "Infrastructure/Road.h"
#include "Infrastructure/Intersection.h"
#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
#include "Vehicle/Motorbike.h"
 
using namespace std;
 
// ---------------------------------------------------------------------------
// Phan 1: dung kich ban
// ---------------------------------------------------------------------------
 
// Mot kich ban hoan chinh. Luu thanh struct de ham main() (va sau nay giao dien) chi can doc no.
struct Scenario {
    unique_ptr<Simulation> sim;     // so huu toan bo Road va Intersection
    vector<Lane*> inLanes;          // 4 lan vao, thu tu: Bac, Nam, Dong, Tay (KHONG so huu)
    Intersection* crossing;         // nut giao chinh, de doc mau den (KHONG so huu)
    int totalVehicles;              // tong so xe luc bat dau
};
 
static const char* DIR_NAMES[4] = {"Bac", "Nam", "Dong", "Tay"};
static const double ROAD_LENGTH = 400.0;   // met
static const double CAR_SPACING = 30.0;    // khoang cach ban dau giua 2 xe lien tiep (m)
 
// n = so xe moi huong; green, yellow = thoi gian den (giay).
// Den do duoc TINH RA = green + yellow de hai nhom huong xen ke khit nhau:
//   nhom Bac-Nam  : phaseOffset = 0      (bat dau bang do)
//   nhom Dong-Tay : phaseOffset = green + yellow  (da "tua" qua het pha do, nen bat dau bang xanh)
// Khi nhom nay xanh/vang thi nhom kia do. Intersection KHONG tu kiem tra xung dot,
// nen viec den lech pha nay chinh la thu bao dam hai huong cat nhau khong cung di.
Scenario buildScenario(int n, double green, double yellow) {
    Scenario sc;
    sc.sim = make_unique<Simulation>(1.0);   // 1 buoc = 1 giay mo phong
    sc.totalVehicles = 0;
 
    const double red = green + yellow;
 
    auto crossing = make_unique<Intersection>(1, "Nga-tu");
    auto exits    = make_unique<Intersection>(2, "Loi-ra");   // don xe o cuoi duong ra va huy chung
 
    int vehicleId = 1;
    for (int i = 0; i < 4; i++) {
        // Xe di THANG: tu huong i sang huong doi dien. Huong doi dien cua i la (i ^ 1): Bac<->Nam, Dong<->Tay.
        auto inRoad  = make_unique<Road>(i + 1, string("vao-") + DIR_NAMES[i], ROAD_LENGTH);
        auto outRoad = make_unique<Road>(i + 5, string("ra-")  + DIR_NAMES[i ^ 1], ROAD_LENGTH);
        Lane* inLane  = inRoad->addLane(i + 1);    // Lane* chi de nhin; Road moi la chu
        Lane* outLane = outRoad->addLane(i + 5);
 
        // Dat xe vao lan vao: xe dau doan cach vach dung 50m, cac xe sau cach nhau CAR_SPACING.
        for (int k = 0; k < n; k++) {
            unique_ptr<Vehicle> v;
            double target;
            switch (k % 3) {
                case 0:  v = make_unique<Car>(vehicleId, 0, 100);          target = 20; break;
                case 1:  v = make_unique<Bus>(vehicleId, 0, 200, 30);      target = 15; break;
                default: v = make_unique<Motorbike>(vehicleId, 0, 125, 100); target = 18; break;   // (id, toc do, phan khoi, nhien lieu)
            }
            vehicleId++;
            v->setTargetSpeed(target);                          // dat TRUOC khi giao xe cho Lane
            inLane->addVehicle(move(v), ROAD_LENGTH - 50.0 - CAR_SPACING * k);   // tu day Lane so huu xe
            sc.totalVehicles++;
        }
 
        // Moi huong vao co den rieng. Nhom Dong-Tay (i = 2, 3) lech pha.
        double offset = (i >= 2) ? green + yellow : 0.0;
        crossing->addApproach(inLane, outLane, red, green, yellow, offset);
 
        // Cuoi duong ra: outgoing = nullptr nghia la xe roi khoi he thong.
        // addApproach luon tao den, nen dung den "xanh gan nhu vinh vien" de xe khong phai dung.
        exits->addApproach(outLane, nullptr, 0.001, 1e9, 0.001);
 
        sc.inLanes.push_back(inLane);
        sc.sim->addRoad(move(inRoad));     // tu day Simulation so huu Road
        sc.sim->addRoad(move(outRoad));
    }
 
    sc.crossing = crossing.get();                       // giu con tro de doc den truoc khi giao quyen so huu
    sc.sim->addIntersection(move(crossing));
    sc.sim->addIntersection(move(exits));
    return sc;
}
 
// ---------------------------------------------------------------------------
// Phan 2: nhap lieu va in ket qua
// ---------------------------------------------------------------------------
 
// Doc 1 so tu ban phim. Bam Enter (hoac het du lieu) -> dung gia tri mac dinh.
double readNumber(const string& prompt, double def, double lo, double hi, bool wholeNumber) {
    while (true) {
        cout << prompt << " [mac dinh " << def << ", tu " << lo << " den " << hi << "]: ";
        string line;
        if (!getline(cin, line) || line.empty()) return def;
        try {
            size_t used = 0;
            double v = stod(line, &used);
            bool ok = (used == line.size()) && v >= lo && v <= hi && (!wholeNumber || v == floor(v));
            if (ok) return v;
        } catch (...) {}
        cout << "Gia tri khong hop le, nhap lai.\n";
    }
}
 
const char* colorName(LightColor c) {
    switch (c) {
        case LightColor::Red:    return "DO  ";
        case LightColor::Green:  return "XANH";
        case LightColor::Yellow: return "VANG";
    }
    return "?   ";
}
 
// 1 dong cho moi buoc. Den cua Bac dai dien nhom Bac-Nam, den cua Dong dai dien nhom Dong-Tay.
void printStatus(const Scenario& sc) {
    Simulation& sim = *sc.sim;
    int onRoad = sim.getVehicleCount();
    cout << "t=" << setw(3) << static_cast<int>(sim.getTime()) << "s"
         << " | Bac-Nam: " << colorName(sc.crossing->getLight(0)->getColor())
         << "  Dong-Tay: " << colorName(sc.crossing->getLight(2)->getColor())
         << " | lan vao: ";
    int inQueue = 0;
    for (int i = 0; i < 4; i++) {
        int c = sc.inLanes[i]->getVehicleCount();
        inQueue += c;
        cout << DIR_NAMES[i][0] << "=" << setw(2) << c << " ";
    }
    // onRoad = xe con trong he thong (lan vao + lan ra). Xe da roi he thong = tong - onRoad.
    cout << "| lan ra: " << setw(2) << (onRoad - inQueue)
         << " | da roi di: " << setw(2) << (sc.totalVehicles - onRoad) << "/" << sc.totalVehicles << "\n";
}
 
int main() {
    cout << "=== MO PHONG NGA TU 4 HUONG ===\n";
    int    n      = static_cast<int>(readNumber("So xe moi huong", 4, 1, 12, true));
    double green  = readNumber("Thoi gian den xanh (giay)", 10, 3, 60, false);
    double yellow = readNumber("Thoi gian den vang (giay)", 3, 1, 10, false);
    int    steps  = static_cast<int>(readNumber("So giay mo phong", 90, 1, 1000, true));
    cout << "(Den do = xanh + vang = " << green + yellow << "s, de hai nhom huong xen ke nhau)\n\n";
 
    Scenario sc = buildScenario(n, green, yellow);
 
    printStatus(sc);                 // trang thai ban dau (t = 0)
    for (int s = 0; s < steps; s++) {
        sc.sim->step();              // 1 buoc: cac Road chay song song, roi Intersection chuyen xe
        printStatus(sc);
    }
    cout << "\nXong. Thoi gian mo phong: " << sc.sim->getTime() << "s, so thread cho xe moi buoc: "
         << sc.sim->getWorkerCount() << "\n";
    return 0;
}
 