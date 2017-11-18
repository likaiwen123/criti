//
// Created by xaq on 9/2/17.
//

#include "acedata.h"
#include "global_fun.h"

void get_nuc_tot_fis_cs(acedata_t *obj, nuclide_t *nuc, nuclide_t *sab_nuc, double erg, double cell_tmp){
//    enum eAdjustCsByPTOrNot{NotAdjustCsByPT = 0, AdjustCsByPT = 1};

    int nNE = GetErgGridNum(nuc);

    /////////////////////// Cal NU /////////////////////////////
    nuc->nu = get_total_nu(nuc, erg);

    //////// binary search for Interpolation parameters /////////
    int min, max;
    min = 1;
    max = GetErgGridNum(nuc);

    get_intplt_pos_fr_double(nuc->XSS, erg, min, max, &nuc->inter_pos, &nuc->inter_frac);

//    if(use_ptable){
//        nuc->prob_table_flag = NotAdjustCsByPT;
//        int nL = GetLocOfLUNR(nuc);
//        if(nL != 0){ // probability tables exist
//            if(erg > nuc->XSS[nL + 6] && erg < nuc->XSS[nL + 5 + (int) (nuc->XSS[nL])]){
//                treat_URR(nuc, erg);
//                nuc->prob_table_flag = AdjustCsByPT;
//                return;
//            }
//        }
//    }

    //tot_cs:
    if(!sab_nuc){
        nuc->tot = INTPLT_BY_POS_FR(nuc->XSS, nuc->inter_pos + nNE, nuc->inter_frac);
        nuc->fis = INTPLT_BY_POS_FR(nuc->fis_XSS, nuc->inter_pos, nuc->inter_frac);
        dpplr_brdn_nuc_tot_cs(obj, nuc, cell_tmp, erg);
        return;
    }

    ////////////////  sab case ///////////////////
    interpolate_sab(nuc, sab_nuc, erg);
}

void dpplr_brdn_nuc_tot_cs(acedata_t *obj, nuclide_t *nuc, double cell_tmp, double erg){
    double a, b, a2, f;
    int bi, j;
    if(nuc->broaden_tmp == ZERO && nuc->broaden_tmp != cell_tmp){ //th_ad:  thermal adjustment
        a2 = nuc->atom_wgt * erg;
        if(a2 <= 500.0 * cell_tmp){
            if(a2 >= 4.0 * cell_tmp)
                f = HALF * cell_tmp / a2;
            else{
                a = sqrt(a2 / cell_tmp);
                b = 25. * a;
                bi = (int) (b);
                f = (obj->therm_func[bi] + (b - bi) * (obj->therm_func[bi + 1] - obj->therm_func[bi])) / a - ONE;
            }
            j = nuc->inter_pos + 3 * GetErgGridNum(nuc);
            nuc->tot += f * (nuc->XSS[j] + nuc->inter_frac * (nuc->XSS[j + 1] - nuc->XSS[j]));
        }
    }
}