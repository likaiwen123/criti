//
// Created by xaq on 10/27/17.
//

#include "criticality.h"
#include "particle_state.h"
#include "neutron_transport.h"

extern particle_state_t base_par_state;

extern criti_t base_criti;

void track_history(){
    if(base_par_state.is_killed) return;

    /* 粒子在当前输运过程中发生碰撞的次数 */
    base_criti.col_cnt = 0;

    do{
        /* geometry tracking: free flying */
        geometry_tracking();
        if(base_par_state.is_killed) break;

        /* sample collision nuclide */
        sample_col_nuclide();
        if(base_par_state.is_killed) break;

        /* calculate cross-section */
        calc_col_nuc_cs();

        /* treat fission */
        treat_fission();

        /* implicit capture(including fission) */
        treat_implicit_capture();
        if(base_par_state.is_killed) break;

        /* sample collision type */
        base_par_state.collision_type = sample_col_type();
        if(base_par_state.is_killed) break;

        /* sample exit state */
        get_exit_state();
        if(base_par_state.is_killed) break;

        /* update particle state */
        base_par_state.erg = base_par_state.exit_erg;
        for(int i = 0; i < 3; i++)
            base_par_state.dir[i] = base_par_state.exit_dir[i];
        double length = ONE / sqrt(SQUARE(base_par_state.dir[0]) +
                                   SQUARE(base_par_state.dir[1]) +
                                   SQUARE(base_par_state.dir[2]));
        base_par_state.dir[0] *= length;
        base_par_state.dir[1] *= length;
        base_par_state.dir[2] *= length;

    } while(++base_criti.col_cnt < MAX_ITER);

    base_criti.tot_col_cnt += base_criti.col_cnt;
}