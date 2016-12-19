#include "mainloop.h"
#include <prof.h>

void mainloop(struct mainloop_data* loop_data)
{
    timepoint_t next_update = millisecs();
    const float skip_ticks = 1000 / loop_data->updates_per_second;

    int loops;
    float interpolation;

    while (!loop_data->should_terminate) {
        loops = 0;
        while (millisecs() > next_update && loops < loop_data->max_frameskip) {
            loop_data->update_callback(loop_data->userdata, skip_ticks);
            next_update += skip_ticks;
            ++loops;
        }

        interpolation = (millisecs() + skip_ticks - next_update) / skip_ticks;
        loop_data->render_callback(loop_data->userdata, interpolation);
    }
}
