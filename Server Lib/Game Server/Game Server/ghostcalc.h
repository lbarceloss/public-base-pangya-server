
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

typedef struct GhostCalcInput {
    double auxpart;
    double mascot;
    double card;
    double ps_card;
    int32_t player_pwr_slot;

    char club_name[16];
    char shot_name[16];
    char ps_name[24];

    double distancia;
    double altura;
    double vento;
    double angulo_vento;
    double terreno;
    double spin;
    double curva;
    double slope_break;

    double res_width;
    double res_height;
    int32_t auto_fit;
    double smart_dev_limit;
} GhostCalcInput;

typedef struct GhostCalcOutput {
    int32_t found;
    double power_percent;
    double power_yards;
    double desvio_pb_real;
    double desvio_pb_raw;
    char smart_desvio[64];
} GhostCalcOutput;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
