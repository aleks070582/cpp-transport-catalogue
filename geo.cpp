#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

    namespace geo
    {
        struct Coordinates {
            double lat;
            double lng;
            bool operator==(const Coordinates& other) const {
                return lat == other.lat && lng == other.lng;
            }
            bool operator!=(const Coordinates& other) const {
                return !(*this == other);
            }
        };

        inline double ComputeDistance(Coordinates from, Coordinates to) {
            using namespace std;
            if (from == to) {
                return 0;
            }


            static const int r_earth = 6371000;
            static const double dr = 3.1415926535 / 180.;
            return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
                * r_earth;
        }
    }
}

  // namespace geo