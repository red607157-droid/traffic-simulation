#undef NDEBUG   // luon bat assert, ke ca khi build voi -DNDEBUG (neu khong, test se "dat" ma khong kiem tra gi)
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include "Traffic/TrafficLight.h"
#include "Traffic/TrafficLightState.h"
#include "Traffic/RedState.h"
#include "Traffic/GreenState.h"
#include "Traffic/YellowState.h"
#include "Integration/LightMapper.h"
#include "Infrastructure/Lane.h"
using namespace std;

bool closeTo(double a, double b) { return fabs(a - b) < 1e-9; }

// Kiem tra mau + ten + thoi gian con lai cua mot den trong 1 dong.
void expectState(const TrafficLight& l, LightColor color, const string& name, double remaining) {
    assert(l.getColor() == color);
    assert(l.getStateName() == name);
    assert(closeTo(l.getTimeRemaining(), remaining));
}

bool ctorThrows(double r, double g, double y) {
    try { TrafficLight l(1, r, g, y); }
    catch (const invalid_argument&) { return true; }
    return false;
}

int main() {
    const double NaN = numeric_limits<double>::quiet_NaN();
    const double INF = numeric_limits<double>::infinity();

    // ---------- Phan A: TrafficLight (context) ----------

    // 1. Thoi luong <= 0 hoac NaN bi tu choi o BAT KY vi tri nao (do, xanh, vang)
    assert(ctorThrows(0, 5, 2) && ctorThrows(5, 0, 2) && ctorThrows(5, 5, 0));
    assert(ctorThrows(-1, 5, 2) && ctorThrows(5, -1, 2) && ctorThrows(5, 5, -1));
    assert(ctorThrows(NaN, 5, 2) && ctorThrows(5, NaN, 2) && ctorThrows(5, 5, NaN));
    assert(!ctorThrows(5, 5, 2));

    // 2. Luon bat dau bang DO, voi du thoi luong do
    {
        TrafficLight l(1, 3, 2, 1);
        expectState(l, LightColor::Red, "RED", 3);
    }

    // 3. Thoi gian con lai giam dung dt; chua het thi chua doi mau
    {
        TrafficLight l(1, 3, 2, 1);
        l.update(1);
        expectState(l, LightColor::Red, "RED", 2);
        l.update(1.5);
        expectState(l, LightColor::Red, "RED", 0.5);
    }

    // 4. Ranh gioi: sat het van con mau cu; het DUNG luc thi doi mau (timer <= 0 la het)
    {
        TrafficLight l(1, 3, 2, 1);
        l.update(2.999);
        assert(l.getColor() == LightColor::Red);
        assert(l.getTimeRemaining() > 0.0 && l.getTimeRemaining() < 0.0011);
        l.update(0.001);                                   // du 3 giay -> sang xanh voi du 2 giay
        assert(l.getColor() == LightColor::Green);
        assert(closeTo(l.getTimeRemaining(), 2));
    }

    // 5. Thu tu DO -> XANH -> VANG -> DO, lap lai nhieu chu ky, moi lan di dung bang thoi luong cua mau do
    {
        TrafficLight l(1, 3, 2, 1);
        for (int cycle = 0; cycle < 4; cycle++) {
            expectState(l, LightColor::Red, "RED", 3);
            l.update(3);
            expectState(l, LightColor::Green, "GREEN", 2);
            l.update(2);
            expectState(l, LightColor::Yellow, "YELLOW", 1);
            l.update(1);
        }
        expectState(l, LightColor::Red, "RED", 3);
    }

    // 6. dt <= 0 va NaN bi bo qua (khong doi mau, khong doi thoi gian con lai)
    {
        TrafficLight l(1, 3, 2, 1);
        l.update(1);
        l.update(0);
        l.update(-5);
        l.update(NaN);
        expectState(l, LightColor::Red, "RED", 2);
    }

    // 7. dt = vo cuc bi bo qua (truoc day vong while chay mai mai -> treo chuong trinh)
    {
        TrafficLight l(1, 3, 2, 1);
        l.update(INF);
        expectState(l, LightColor::Red, "RED", 3);
    }

    // 8. Thoi gian du (overshoot) duoc mang sang mau ke tiep
    {
        TrafficLight l(1, 3, 2, 1);
        l.update(4);                                       // het do (3), du 1 -> xanh con 2 - 1 = 1
        expectState(l, LightColor::Green, "GREEN", 1);
    }

    // 9. dt lon di qua NHIEU mau trong 1 lan goi
    {
        TrafficLight l(1, 3, 2, 1);
        l.update(5.5);                                     // do (3) + xanh (2) + 0.5 vang -> vang con 0.5
        expectState(l, LightColor::Yellow, "YELLOW", 0.5);

        TrafficLight m(2, 3, 2, 1);
        m.update(6);                                       // dung 1 chu ky -> ve DO voi du 3 giay
        expectState(m, LightColor::Red, "RED", 3);

        TrafficLight n(3, 3, 2, 1);
        n.update(10);                                      // 6 + 3 + 1: het 1 chu ky + het do, xanh da qua 1 giay
        expectState(n, LightColor::Green, "GREEN", 1);
    }

    // 10. Nhieu buoc nho == mot buoc lon (dt la luy thua cua 2 nen cong don khong bi sai so)
    {
        TrafficLight small(1, 12, 10, 3), big(2, 12, 10, 3);
        for (int i = 0; i < 100; i++) small.update(0.25);  // 25 giay
        big.update(25);
        assert(small.getColor() == big.getColor());
        assert(closeTo(small.getTimeRemaining(), big.getTimeRemaining()));
        assert(small.getColor() == LightColor::Red);       // 12 + 10 + 3 = 25 -> vua ve DO
        assert(closeTo(small.getTimeRemaining(), 12));
    }

    // 11. Thoi luong khong nguyen
    {
        TrafficLight l(1, 1.5, 0.5, 0.25);
        l.update(1.5);
        expectState(l, LightColor::Green, "GREEN", 0.5);
        l.update(0.5);
        expectState(l, LightColor::Yellow, "YELLOW", 0.25);
        l.update(0.25);
        expectState(l, LightColor::Red, "RED", 1.5);
    }

    // 12. Kich ban giong main.cpp (do 12, xanh 10, vang 3): khop voi test_simulation (buoc 11/12/22/25)
    {
        TrafficLight l(1, 12, 10, 3);
        l.update(11);
        assert(l.getColor() == LightColor::Red);
        l.update(1);
        assert(l.getColor() == LightColor::Green);
        l.update(10);
        assert(l.getColor() == LightColor::Yellow);
        l.update(3);
        assert(l.getColor() == LightColor::Red);
    }

    // 13. Bat bien khi chay dai: sau MOI update, den chua het gio (con > 0) va khong vuot thoi luong cua mau hien tai;
    //     ti le thoi gian moi mau xap xi ti le thoi luong (5 : 4 : 2)
    {
        const double dur[3] = {5, 4, 2};                   // do, xanh, vang
        TrafficLight l(1, dur[0], dur[1], dur[2]);
        int count[3] = {0, 0, 0};
        const int N = 22000;                               // 22000 * 0.1s = 2200s = 200 chu ky (11s)
        for (int i = 0; i < N; i++) {
            l.update(0.1);
            int idx = l.getColor() == LightColor::Red ? 0 : (l.getColor() == LightColor::Green ? 1 : 2);
            count[idx]++;
            assert(l.getTimeRemaining() > 0.0);
            assert(l.getTimeRemaining() <= dur[idx] + 1e-9);
        }
        for (int c = 0; c < 3; c++) {
            double fraction = static_cast<double>(count[c]) / N;
            assert(fabs(fraction - dur[c] / 11.0) < 0.01);
        }
    }

    // 14. Hai den doc lap: update den nay khong lam doi den kia (moi den co ban sao config rieng)
    {
        TrafficLight a(1, 3, 2, 1), b(2, 10, 10, 10);
        a.update(3);
        expectState(a, LightColor::Green, "GREEN", 2);
        expectState(b, LightColor::Red, "RED", 10);
    }

    // 15. display() in ra id, ten mau, thoi gian con lai
    {
        ostringstream captured;
        streambuf* old = cout.rdbuf(captured.rdbuf());
        TrafficLight l(7, 3, 2, 1);
        l.display();
        cout.rdbuf(old);
        string out = captured.str();
        assert(out.find("TrafficLight no.7") != string::npos);
        assert(out.find("RED") != string::npos);
        assert(out.find("time left: 3") != string::npos);
    }

    // ---------- Phan B: cac State (State Pattern) ----------

    // 16. Moi state: dung mau, dung ten, bat dau voi dung thoi luong cua minh trong config
    {
        TrafficLightConfig cfg{3, 2, 1};
        RedState r(cfg); GreenState g(cfg); YellowState y(cfg);
        assert(r.getColor() == LightColor::Red && r.getName() == "RED" && closeTo(r.getTimeRemaining(), 3));
        assert(g.getColor() == LightColor::Green && g.getName() == "GREEN" && closeTo(g.getTimeRemaining(), 2));
        assert(y.getColor() == LightColor::Yellow && y.getName() == "YELLOW" && closeTo(y.getTimeRemaining(), 1));
        assert(!r.isFinished() && !g.isFinished() && !y.isFinished());
    }

    // 17. State tu dem nguoc; dt am bi bo qua; isFinished khi timer <= 0 (timer co the am = thoi gian du)
    {
        TrafficLightConfig cfg{3, 2, 1};
        RedState r(cfg);
        r.update(-1);
        assert(closeTo(r.getTimeRemaining(), 3));
        r.update(2);
        assert(!r.isFinished() && closeTo(r.getTimeRemaining(), 1));
        r.update(1);
        assert(r.isFinished() && closeTo(r.getTimeRemaining(), 0));
        r.update(0.5);
        assert(r.isFinished() && closeTo(r.getTimeRemaining(), -0.5));   // -timer = overshoot
    }

    // 18. nextState(): DO -> XANH -> VANG -> DO; state moi bat dau day thoi luong va giu nguyen config
    {
        TrafficLightConfig cfg{3, 2, 1};
        unique_ptr<TrafficLightState> s = make_unique<RedState>(cfg);
        const LightColor expected[] = {LightColor::Green, LightColor::Yellow, LightColor::Red, LightColor::Green};
        const double expectedTime[] = {2, 1, 3, 2};
        for (int i = 0; i < 4; i++) {
            s = s->nextState();
            assert(s->getColor() == expected[i]);
            assert(closeTo(s->getTimeRemaining(), expectedTime[i]));
        }
    }

    // ---------- Phan C: tich hop voi Vehicle / Lane ----------

    // 19. LightMapper: moi mau cua den -> dung tin hieu Vehicle::trafficLight
    {
        TrafficLight l(1, 3, 2, 1);
        assert(LightMapper(l) == Vehicle::redLight);
        l.update(3);
        assert(LightMapper(l) == Vehicle::greenLight);
        l.update(2);
        assert(LightMapper(l) == Vehicle::yellowLight);
        l.update(1);
        assert(LightMapper(l) == Vehicle::redLight);
    }

    // 20. Lane doc den SONG: khong co den -> unknownLight (an toan = coi nhu do); co den -> theo mau hien tai
    {
        Lane lane(1, "L", 100.0);
        assert(lane.getLightSignal() == Vehicle::unknownLight);
        TrafficLight l(1, 3, 2, 1);
        lane.setTrafficLight(&l);
        assert(lane.getLightSignal() == Vehicle::redLight);
        l.update(3);
        assert(lane.getLightSignal() == Vehicle::greenLight);   // Lane chi giu con tro, khong sao chep mau
        l.update(2);
        assert(lane.getLightSignal() == Vehicle::yellowLight);
    }

    cout << "Tat ca test traffic light dat\n";
    return 0;
}
