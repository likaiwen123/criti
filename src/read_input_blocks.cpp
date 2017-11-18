//
// Created by xaq on 10/25/17.
//

#include "IO_releated.h"

extern IOfp_t base_IOfp;

extern CALC_MODE_T base_mode;

/* -------------------------- private prototypes ---------------------------- */
int _identify_kw(char *kw);

/* ----------------------------- API implementation ------------------------- */
void read_input_blocks(){
    char buf[256];
    char *ret;
    char *kw_start;

    while((ret = fgets(buf, MAX_LINE_LENGTH, base_IOfp.inp_fp)) != nullptr){
        /* find the first non-space character */
        while(ISSPACE(*ret)) ret++;

        /* comment line or blank line */
        if(ISCOMMENT(*ret) || ISRETURN(*ret)) continue;

        if(ISALPHA(*ret)){
            kw_start = ret;

            while(ISALPHA(*ret)) {
                *ret = TOUPPER(*ret);
                ret++;
            }
            *ret = 0;

            /* process all cases depending on key words */
            switch(_identify_kw(kw_start)){
                case 0:    /* UNIVERSE */
                    ret++;
                    read_universe_block(ret);
                    break;
                case 1:    /* SURFACE */
                    read_surf_block();
                    break;
                case 2:    /* MATERIAL */
                    read_material_block();
                    break;
                case 3:    /* CRITICALITY */
                    base_mode = CRITICALITY;
                    read_criticality_block();
                    break;
                case 4:    /* TALLY */
//                    read_tally_block();
                    break;
                case 5:    /* FIXEDSOURCE */
                    base_mode = FIXEDSOURCE;
//                    read_fixed_src_block();
                case 6:    /* DEPLETION */
                    base_mode = POINTBURN;
                    break;
                case 7:    /* BURNUP */
                    base_mode = BURNUP;
//                    read_burnup_block();
                    break;
                default:
                    printf("unknown key word %s.\n", kw_start);
                    break;
            }
        }
    }

//    check_input_block();

    fclose(base_IOfp.inp_fp);
}

/* ------------------------ private API implementation ---------------------- */
int _identify_kw(char *kw){
    for(int i = 0; i < KW_NUMBER; i++){
        if(strcmp(kw, keyword[i]) == 0)
            return i;
    }
    return -1;
}