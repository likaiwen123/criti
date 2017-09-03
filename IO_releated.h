//
// Created by xaq on 9/2/17.
//

#ifndef TTYW_IO_RELEATED_H
#define TTYW_IO_RELEATED_H

#include "common.h"

typedef struct {
    FILE *inp_fp;
    FILE *opt_fp;
    FILE *mat_fp;
    FILE *tally_fp;

    char inp_file_name[MAX_FILENAME_LENGTH];
    char opt_file_name[MAX_FILENAME_LENGTH];
} IOfp;

extern IOfp *base_IOfp;

#ifdef __cplusplus
extern "C" {
#endif

void check_IO_file(int, char **);
void read_input_blocks();
void read_universe_block();
void read_cell_card();
void read_cell_surf_bool();
void read_surf_block();
void read_material_block();
void read_mat_card();
void read_sab_card();
void read_criticality_block();
void read_fixed_src_block();
void read_tally_block();
void read_cell_tally_card();
void read_mesh_tally_card();
void read_tally_cells();
void read_cs_tally_card();

void output_heading();
void output_ending();

#ifdef __cplusplus
}
#endif

#endif //TTYW_IO_RELEATED_H
