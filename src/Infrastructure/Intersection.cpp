#include <cmath>
#include <iostream>
#include <stdexcept>
#include "Infrastructure/Intersection.h"

using namespace std;

Intersection::Intersection(int _id, const string& _name, double _minEntryGap)
    : RoadComponent(_id, _name), minEntryGap(_minEntryGap) {
    if (!(_minEntryGap >= 0.0)) throw invalid_argument("Intersection: minEntryGap phai >= 0");
}

TrafficLight* Intersection::addApproach(Lane* incoming, Lane* outgoing,
                                        double redDur, double greenDur, double yellowDur,
                                        double phaseOffset) {
    if (incoming == nullptr || incoming == outgoing || incoming->hasTrafficLight()) return nullptr;
    if (!(phaseOffset >= 0.0) || !isfinite(phaseOffset)) throw invalid_argument("Intersection: phaseOffset phai la so huu han >= 0");

    // Tạo đèn trước (có thể ném exception nếu thời lượng sai) khi chưa thay đổi gì.
    auto light = make_unique<TrafficLight>(incoming->getId(), redDur, greenDur, yellowDur);
    if (phaseOffset > 0.0) light->update(phaseOffset);

    TrafficLight* raw = light.get();
    incoming->setTrafficLight(raw);
    approaches.push_back(Approach{incoming, outgoing, move(light)});
    return raw;
}

size_t Intersection::getApproachCount() const { return approaches.size(); }
TrafficLight* Intersection::getLight(size_t index) const {
    return index < approaches.size() ? approaches[index].light.get() : nullptr;
}

void Intersection::update(double dt) {
    if (!(dt > 0.0)) return;
    for (auto& a : approaches) {
        a.light->update(dt);

        // Có thể có nhiều xe cùng vượt vạch trong 1 tick -> lặp tới khi hết xe hoặc làn ra đầy.
        while (a.incoming->hasExitedVehicle()) {
            if (a.outgoing == nullptr) {                       // rời khỏi hệ thống
                a.incoming->takeExitedVehicle();               // unique_ptr trả về bị huỷ ngay
                continue;
            }
            if (!a.outgoing->hasRoomAtEntry(minEntryGap)) break;   // xe vẫn ở lại làn vào, chờ tick sau

            unique_ptr<Vehicle> v = a.incoming->takeExitedVehicle();
            double overshoot = v->getPosition() - a.incoming->getLength();   // quãng đã đi quá vạch trong tick này
            a.outgoing->addVehicle(move(v), overshoot);
        }
    }
}

void Intersection::display() const {
    cout << "Intersection no." << getId() << " (" << getName() << ") - approaches: " << approaches.size() << "\n";
    for (const auto& a : approaches) {
        cout << "  Lane " << a.incoming->getId() << " -> ";
        if (a.outgoing) cout << "Lane " << a.outgoing->getId(); else cout << "EXIT";
        cout << " | ";
        a.light->display();
    }
}

int Intersection::getVehicleCount() const { return 0; }