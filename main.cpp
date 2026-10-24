// main.cpp - Mo phong NGA TU (4 huong) hoac NGA BA (3 huong).
//
// Chuong trinh nay CHI dung nhung gi nhom da co (Road, Lane, Intersection, TrafficLight, Vehicle, Simulation),
// khong sua file nao khac.
//
// Moi huong vao gom: 1 Road "vao" (xe chay toi vach dung o cuoi duong, tai position == length)
// va xe sau khi qua den se sang 1 Road "ra" (toi cuoi duong thi roi khoi he thong).
//
// Ai so huu ai (xem phan giai thich Composite):
//   Simulation --so huu--> Road --so huu--> Lane --so huu--> Vehicle
//   Simulation --so huu--> Intersection --so huu--> TrafficLight
// Cac con tro Lane* / Intersection* trong Scenario chi de NHIN, khong phai chu so huu.
 
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
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
// Phan 1: mo ta kieu nut giao (du lieu, khong phai code xu ly)
// ---------------------------------------------------------------------------
 
// 4 huong co the co: chi so 0..3
static const char* DIR_NAMES[4] = {"Bac", "Nam", "Dong", "Tay"};
static const double ROAD_LENGTH = 400.0;   // met
static const double CAR_SPACING = 30.0;    // khoang cach ban dau giua 2 xe lien tiep (m)
// Khoang trong toi thieu o dau lan ra thi nut giao moi cho xe moi vao (tham so minEntryGap cua Intersection).
// Mac dinh cua nhom la 5m, nhung xe moi vao co the dang chay 12-18 m/s con xe truoc cham hon, nen chi cach 1m.
// 20m giu khoang cach giua 2 xe >= 5m (bang safeDistance nho nhat) doi lai xe thoat cham hon khoang 20%.
static const double ENTRY_GAP   = 20.0;
 
// Mot huong vao: xe di tu huong `from`, qua den, roi sang duong ra ve huong `to`.
//   from == huong doi dien cua to  -> xe di THANG
//   khac                           -> xe RE (voi thiet ke cua nhom, "re" chi la chon lan ra khac)
// phase: 0 hoac 1. Cac huong CUNG phase xanh cung luc; hai phase khac nhau xen ke (khong bao gio cung xanh).
struct ApproachSpec {
    int from;
    int to;
    int phase;
};
 
struct Layout {
    vector<ApproachSpec> approaches;
    string phaseName[2];    // nhan hien thi cho 2 phase
};
 
// arms = 4: nga tu.  arms = 3: nga ba (chu T).
Layout makeLayout(int arms) {
    Layout L;
    if (arms == 4) {
        // Bac<->Nam di thang (phase 0), Dong<->Tay di thang (phase 1).
        L.approaches = { {0, 1, 0}, {1, 0, 0}, {2, 3, 1}, {3, 2, 1} };
        L.phaseName[0] = "Bac-Nam";
        L.phaseName[1] = "Dong-Tay";
    } else {
        // Nga ba: duong chinh Bac-Nam thang (phase 0); nhanh Dong khong co duong doi dien nen RE sang Nam (phase 1).
        // Hai huong Bac va Dong cung do ve lan ra "ra-Nam": chinh la cho hai dong xe nhap lai.
        L.approaches = { {0, 1, 0}, {1, 0, 0}, {2, 1, 1} };
        L.phaseName[0] = "Bac-Nam";
        L.phaseName[1] = "Dong";
    }
    return L;
}
 
// ---------------------------------------------------------------------------
// Phan 2: dung kich ban
// ---------------------------------------------------------------------------
 
// Mot kich ban hoan chinh. Luu thanh struct de ham main() (va sau nay giao dien) chi can doc no.
struct Scenario {
    Layout layout;
    unique_ptr<Simulation> sim;     // so huu toan bo Road va Intersection
    vector<Lane*> inLanes;          // lan vao, cung thu tu voi layout.approaches (KHONG so huu)
    Lane* outLanes[4];              // lan ra theo huong 0..3; nullptr neu huong do khong co (KHONG so huu)
    Intersection* crossing;         // nut giao chinh, de doc mau den (KHONG so huu)
    int totalVehicles;              // tong so xe luc bat dau
    map<int, size_t> approachOf;    // id xe -> chi so huong vao ban dau (de bao cao xe di thang hay re)
    set<int> crossed;               // id cac xe da bao cao la qua nut giao (de moi xe chi bao 1 lan)
};
 
// arms = 3 hoac 4; n = so xe moi huong; green, yellow = thoi gian den (giay).
// Den do duoc TINH RA = green + yellow de hai phase xen ke khit nhau:
//   phase 0 : phaseOffset = 0              (bat dau bang do)
//   phase 1 : phaseOffset = green + yellow (da "tua" qua het pha do, nen bat dau bang xanh)
// Intersection KHONG tu kiem tra xung dot, nen viec den lech pha nay chinh la thu bao dam
// cac huong cat nhau (hoac cung do vao mot lan ra) khong cung di.
Scenario buildScenario(int arms, int n, double green, double yellow, double minEntryGap = ENTRY_GAP) {
    Scenario sc;
    sc.layout = makeLayout(arms);
    sc.sim = make_unique<Simulation>(1.0);   // 1 buoc = 1 giay mo phong
    sc.totalVehicles = 0;
    for (int d = 0; d < 4; d++) sc.outLanes[d] = nullptr;
 
    const double red = green + yellow;
 
    auto crossing = make_unique<Intersection>(1, "Nut-giao", minEntryGap);
    auto exits    = make_unique<Intersection>(2, "Loi-ra");   // don xe o cuoi duong ra va huy chung
 
    // (a) Tao cac Road RA truoc, moi huong dich 1 road (vi du nga ba: ra-Bac va ra-Nam).
    vector<unique_ptr<Road>> outRoads;
    for (const ApproachSpec& a : sc.layout.approaches) {
        if (sc.outLanes[a.to] != nullptr) continue;   // huong dich nay da co road ra roi (nhieu huong dung chung)
        auto road = make_unique<Road>(10 + a.to, string("ra-") + DIR_NAMES[a.to], ROAD_LENGTH);
        sc.outLanes[a.to] = road->addLane(10 + a.to);   // Lane* chi de nhin; Road moi la chu
        outRoads.push_back(move(road));
    }
 
    // (b) Tao cac Road VAO, dat xe, noi voi nut giao.
    int vehicleId = 1;
    vector<unique_ptr<Road>> inRoads;
    for (size_t i = 0; i < sc.layout.approaches.size(); i++) {
        const ApproachSpec& a = sc.layout.approaches[i];
        auto inRoad = make_unique<Road>(static_cast<int>(i) + 1, string("vao-") + DIR_NAMES[a.from], ROAD_LENGTH);
        Lane* inLane = inRoad->addLane(static_cast<int>(i) + 1);
 
        // Dat xe vao lan vao: xe dau doan cach vach dung 50m, cac xe sau cach nhau CAR_SPACING.
        for (int k = 0; k < n; k++) {
            unique_ptr<Vehicle> v;
            double target;
            switch (k % 3) {
                case 0:  v = make_unique<Car>(vehicleId, 0, 100);            target = 20; break;
                case 1:  v = make_unique<Bus>(vehicleId, 0, 200, 30);        target = 15; break;
                default: v = make_unique<Motorbike>(vehicleId, 0, 125, 100); target = 18; break;   // (id, toc do, phan khoi, nhien lieu)
            }
            sc.approachOf[vehicleId] = i;
            vehicleId++;
            v->setTargetSpeed(target);                          // dat TRUOC khi giao xe cho Lane
            inLane->addVehicle(move(v), ROAD_LENGTH - 50.0 - CAR_SPACING * static_cast<double>(k));   // tu day Lane so huu xe
            sc.totalVehicles++;
        }
 
        // Moi huong vao co den rieng. Phase 1 lech pha.
        double offset = (a.phase == 1) ? green + yellow : 0.0;
        crossing->addApproach(inLane, sc.outLanes[a.to], red, green, yellow, offset);
 
        sc.inLanes.push_back(inLane);
        inRoads.push_back(move(inRoad));
    }
 
    // (c) Cuoi moi duong ra: outgoing = nullptr nghia la xe roi khoi he thong.
    // addApproach luon tao den, nen dung den "xanh gan nhu vinh vien" de xe khong phai dung.
    for (int d = 0; d < 4; d++)
        if (sc.outLanes[d] != nullptr) exits->addApproach(sc.outLanes[d], nullptr, 0.001, 1e9, 0.001);
 
    // (d) Giao quyen so huu cho Simulation. Road truoc, Intersection sau (Simulation cung tu dam bao thu tu nay).
    for (auto& r : inRoads)  sc.sim->addRoad(move(r));
    for (auto& r : outRoads) sc.sim->addRoad(move(r));
    sc.crossing = crossing.get();                       // giu con tro de doc den truoc khi giao quyen so huu
    sc.sim->addIntersection(move(crossing));
    sc.sim->addIntersection(move(exits));
    return sc;
}
 
// ---------------------------------------------------------------------------
// Phan 3: nhap lieu va in ket qua
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
 
static const char* typeName(const Vehicle* v) {
    if (dynamic_cast<const Car*>(v))       return "Car";
    if (dynamic_cast<const Bus*>(v))       return "Bus";
    if (dynamic_cast<const Motorbike*>(v)) return "Motorbike";
    return "Xe";
}
 
// Moi xe moi xuat hien trong mot lan ra = vua qua nut giao. In ra: di tu dau, sang dau, THANG hay RE.
// "Di thang" = lan ra nam o huong DOI DIEN (chi so huong khac nhau o bit cuoi: Bac<->Nam, Dong<->Tay).
void reportCrossings(Scenario& sc) {
    for (int d = 0; d < 4; d++) {
        Lane* lane = sc.outLanes[d];
        if (lane == nullptr) continue;
        for (int i = 0; i < lane->getVehicleCount(); i++) {
            Vehicle* v = lane->getVehicleAt(i);
            if (!sc.crossed.insert(v->getId()).second) continue;   // da bao cao roi
            int from = sc.layout.approaches[sc.approachOf.at(v->getId())].from;
            bool straight = ((from ^ 1) == d);
            cout << "  >> xe #" << setw(2) << v->getId() << " (" << typeName(v) << ") di tu " << DIR_NAMES[from]
                 << " sang " << DIR_NAMES[d] << ": " << (straight ? "DI THANG" : "RE") << "\n";
        }
    }
}
 
// Chi so cua huong dau tien thuoc phase p (den cua huong do dai dien cho ca phase).
static size_t firstOfPhase(const Layout& L, int p) {
    for (size_t i = 0; i < L.approaches.size(); i++)
        if (L.approaches[i].phase == p) return i;
    return 0;
}
 
// 1 dong cho moi buoc.
void printStatus(const Scenario& sc) {
    Simulation& sim = *sc.sim;
    int onRoad = sim.getVehicleCount();
    cout << "t=" << setw(3) << static_cast<int>(sim.getTime()) << "s";
    for (int p = 0; p < 2; p++) {
        LightColor c = sc.crossing->getLight(firstOfPhase(sc.layout, p))->getColor();
        cout << (p == 0 ? " | " : "  ") << sc.layout.phaseName[p] << ": " << colorName(c);
    }
    cout << " | lan vao: ";
    for (size_t i = 0; i < sc.inLanes.size(); i++) {
        int c = sc.inLanes[i]->getVehicleCount();
        cout << DIR_NAMES[sc.layout.approaches[i].from][0] << "=" << setw(2) << c << " ";
    }
    cout << "| lan ra: ";
    for (int d = 0; d < 4; d++)
        if (sc.outLanes[d] != nullptr)
            cout << DIR_NAMES[d][0] << "=" << setw(2) << sc.outLanes[d]->getVehicleCount() << " ";
    // onRoad = xe con trong he thong (lan vao + lan ra). Xe da roi he thong = tong - onRoad.
    cout << "| da roi di: " << setw(2) << (sc.totalVehicles - onRoad) << "/" << sc.totalVehicles << "\n";
}
 
// Xe di THANG khi huong dich la huong DOI DIEN huong vao (Bac<->Nam, Dong<->Tay); nguoc lai la RE.
static bool isStraight(const ApproachSpec& a) { return (a.from ^ 1) == a.to; }
 
// In bang cac tuyen o dau chuong trinh de nguoi xem biet xe nao se di dau.
void printRoutes(const Scenario& sc) {
    cout << "Cac tuyen duong:\n";
    for (const ApproachSpec& a : sc.layout.approaches)
        cout << "  " << setw(4) << left << DIR_NAMES[a.from] << right << " -> " << setw(4) << left << DIR_NAMES[a.to] << right
             << (isStraight(a) ? "  (di thang)" : "  (RE)      ")
             << "  den thuoc phase " << sc.layout.phaseName[a.phase] << "\n";
    cout << "\n";
}
 
int main() {
    cout << "=== MO PHONG NUT GIAO ===\n";
    int    arms   = static_cast<int>(readNumber("Kieu nut giao (4 = nga tu, 3 = nga ba)", 4, 3, 4, true));
    int    n      = static_cast<int>(readNumber("So xe moi huong", 4, 1, 12, true));
    double green  = readNumber("Thoi gian den xanh (giay)", 10, 3, 60, false);
    double yellow = readNumber("Thoi gian den vang (giay)", 3, 1, 10, false);
    int    steps  = static_cast<int>(readNumber("So giay mo phong", 90, 1, 1000, true));
    cout << "(Den do = xanh + vang = " << green + yellow << "s, de cac phase xen ke nhau)\n\n";
 
    Scenario sc = buildScenario(arms, n, green, yellow);
 
    printRoutes(sc);                 // bang cac tuyen: xe tu huong nao di sang huong nao, thang hay re
    printStatus(sc);                 // trang thai ban dau (t = 0)
    for (int s = 0; s < steps; s++) {
        sc.sim->step();              // 1 buoc: cac Road chay song song, roi Intersection chuyen xe
        reportCrossings(sc);         // in cac xe vua qua nut giao (di thang hay re)
        printStatus(sc);
    }
    cout << "\nXong. Thoi gian mo phong: " << sc.sim->getTime() << "s, so thread cho xe moi buoc: "
         << sc.sim->getWorkerCount() << "\n";
    return 0;
}