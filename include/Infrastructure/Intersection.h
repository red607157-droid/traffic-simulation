#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <memory>
#include <string>
#include <vector>
#include "Infrastructure/RoadComponent.h"
#include "Infrastructure/Lane.h"
#include "Traffic/TrafficLight.h"

// Nút giao: SỞ HỮU các đèn giao thông và chuyển xe từ làn vào (incoming) sang làn ra (outgoing).
// Mỗi "Approach" = một hướng đi vào nút giao: làn vào + đèn của làn đó + làn ra kế tiếp.
class Intersection : public RoadComponent {
public:
    struct Approach {
        Lane* incoming;                          // KHÔNG sở hữu (Road sở hữu)
        Lane* outgoing;                          // KHÔNG sở hữu, nullptr = xe rời khỏi hệ thống (bị huỷ)
        std::unique_ptr<TrafficLight> light;     // Intersection sở hữu đèn
    };

private:
    std::vector<Approach> approaches;
    double minEntryGap;   // khoảng trống tối thiểu ở đầu làn ra để cho xe vào (m)

public:
    Intersection(int _id, const std::string& _name, double _minEntryGap = 5.0);

    // Thêm một hướng đi vào. Tạo TrafficLight với thời lượng cho trước, gắn vào làn vào (Lane::setTrafficLight).
    // Trả về con trỏ KHÔNG sở hữu tới đèn vừa tạo; nullptr nếu incoming là nullptr.
    // Lưu ý: đèn phải được gắn TRƯỚC khi mô phỏng chạy; làn vào/ra phải sống lâu hơn Intersection.
    // phaseOffset (giây, >= 0): tua đèn đi trước chừng đó để các hướng không cùng xanh/đỏ một lúc.
    // Trả về nullptr nếu incoming == nullptr, incoming == outgoing, hoặc làn vào đã có đèn.
    // Ném std::invalid_argument nếu thời lượng đèn không > 0 hoặc phaseOffset < 0.
    TrafficLight* addApproach(Lane* incoming, Lane* outgoing,
                              double redDur, double greenDur, double yellowDur,
                              double phaseOffset = 0.0);

    std::size_t getApproachCount() const;
    TrafficLight* getLight(std::size_t index) const;   // nullptr nếu index sai

    // Thứ tự: cập nhật đèn -> với mỗi hướng: nếu xe đầu đoàn đã tới cuối làn vào và làn ra còn chỗ (hasRoomAtEntry) -> đưa sang làn ra;
    // nếu outgoing == nullptr -> huỷ xe (rời hệ thống).
    // Xe không vào được làn ra (đầy) sẽ bị giữ lại ở cuối làn vào, không bị mất.
    void update(double dt) override;
    void display() const override;
    // Nút giao không chứa xe riêng (xe nằm trong Lane) nên luôn trả về 0.
    int getVehicleCount() const override;
};

#endif
