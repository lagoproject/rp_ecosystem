#include <stdio.h>
#include <stdlib.h>
#include "gps_rp.h"

int main(void) {
    // Open
    gps_init();

    loc_t data;

    while (1) {
        gps_location(&data);

        printf("%lf %lf %02d:%02d:%02d %02d/%02d/%04d\n", data.latitude, data.longitude, data.D.hour, data.D.minute, data.D.second, data.D.day, data.D.month,data.D.year);
    }

    return EXIT_SUCCESS;
}

