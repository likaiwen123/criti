//
// Created by xaq on 10/26/17.
//

#include "criticality.h"
#include "RNG.h"
#include "IO_releated.h"


extern criti_t base_criti;

void calc_criticality(){
    init_fission_src();

    particle_state_t par_state;
    for(int cyc = 1; cyc <= base_criti.tot_cycle_num; cyc++){
        for(int neu = 1; neu <= base_criti.cycle_neutron_num; neu++){
            get_rand_seed();

            /* sample source particle */
            sample_fission_src(&par_state);

            /* neutron history */
            track_history(&par_state);
        }
        process_cycle_end(cyc);
    }
    output_summary();
}