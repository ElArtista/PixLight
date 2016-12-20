#include "mainloop.h"
#include <tinycthread.h>
#include <prof.h>

void mainloop(struct mainloop_data* loop_data)
{
    const float skip_ticks = 1000.0f / loop_data->updates_per_second;
    while (!loop_data->should_terminate) {
        timepoint_t t1 = millisecs();
        loop_data->update_callback(loop_data->userdata, skip_ticks);
        loop_data->render_callback(loop_data->userdata, 1.0f);
        timepoint_t t2 = millisecs();
        long long remaining = skip_ticks - (t2 - t1);
        if (remaining > 0) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 1000000 * remaining;
            thrd_sleep(&ts, 0);
        }
    }
}
