//
// Created by xaq on 10/7/17.
//

#ifndef TTYW_UNIVERSE_H
#define TTYW_UNIVERSE_H

#include "common.h"
#include "map.h"
#include "vector.h"

typedef struct {
    vector cells;    /* 存储的是直接的cell_index，而不是某个数组的下标 */

    double origin[3];
    double rotation[3][3];

    bool is_moved;
    bool is_rotated;
    bool is_lattice;
    int lattice_type;
    double pitch[3];
    int scope[3];
    double sita;
    double sin_sita;
    double cos_sita;
    double height;

    int *filled_lat_univs;
    int filled_lat_univs_sz;

    /* 在同一个universe内部，每个cell基于每个面的唯一的一个邻居(cell)的cell_id */
    /* 当前neighbor_lists实现的是map嵌套map，即map<cell_index, map<surface_index, address of neighbor_cell_index>> */
    map *neighbor_lists;
} universe_t;


#define UNIV_KW_NUMBER      7
#define UNIV_MAX_KW_LENGTH  8

static const char universe_kw[UNIV_KW_NUMBER][UNIV_MAX_KW_LENGTH] = {
        "MOVE",
        "ROTATE",
        "LAT",
        "PITCH",
        "SCOPE",
        "SITA",
        "FILL"
};

/////////////////// hexagon lattice ///////////////////////
///             E                                       ///
///             *               / b1 = (L1, 0)          ///
///           *   *            /                        ///
///      F  *       * D       /                         ///
///         *   O   *        /------> b2=(0.5*L1, H)    ///
///      A  *       * C                                 ///
///           *   *                                     ///
///             *            <FA,AB> = sita             ///
///             B                                       ///
///  1: FA ,  2: AB ,  3: BC ,  4: CD , 5: DE ,  6: EF  ///
///////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

universe_t *univ_init();
void trans_univ_coord(universe_t *obj, double pos[3], double dir[3]);
void trans_univ_dir(universe_t *obj, double dir[3]);
int find_lat_index(universe_t *obj, const double pos[3], const double dir[3]);
void move_to_origin_lat(universe_t *obj, int lat_index, double pos[3]);
double calc_dist_to_lat(universe_t *obj, const double pos[3], const double dir[3], int *which_surf);
int offset_neighbor_lat(universe_t *obj, int lat_index, int lat_bound_surf, double pos[3]);
void univ_free(universe_t *obj);

#ifdef __cplusplus
}
#endif

#endif //TTYW_UNIVERSE_H
