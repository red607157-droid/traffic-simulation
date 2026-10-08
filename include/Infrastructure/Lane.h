
#ifndef LANE_H
#define LANE_H

class Lane {
public:
    // Duy viết lại file này nhé
    // code cái này tạm để test thôi
    double getDistanceToNextLight(double vehiclePosition) {
        double lightPosition = 500.0; // giả sử vạch dừng ở 500m
        if (vehiclePosition > lightPosition) return 9999.0;
        return lightPosition - vehiclePosition;
    }
};

#endif