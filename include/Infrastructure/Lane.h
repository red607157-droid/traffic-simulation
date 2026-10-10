
#ifndef LANE_H
#define LANE_H

class Lane {
public:
    // Duy viết lại file này nhé
    // code cái này tạm để test thôi
    double getDistanceToNextLight(double vehiclePosition) const {
        const double lightPosition = 500.0; // giả sử vạch dừng ở 500m
        // dung sai 1e-6: xe dừng đúng vạch có thể lệch 1e-13 do số thực, không được coi là "đã qua vạch"
        if (vehiclePosition > lightPosition + 1e-6) return 9999.0;
        double d = lightPosition - vehiclePosition;
        return d < 0.0 ? 0.0 : d;
    }
};

#endif
