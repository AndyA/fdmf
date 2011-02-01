#include "fdmf_sonic_reducer.h"

/*
This routine calculates some metrics on the band energies of a chunk.
The metrics are written to arrays, with one array for each metric.
The chunk argument is the position in the output arrays where the
metrics should be stored.
*/

void
chunk_metrics( double *be, f_c * e, f_c * r, f_c * t, int chunk ) {
  double energy, ratio, twist, lows, highs, evens, odds;
  lows = be[0] + be[1];
  highs = be[2] + be[3];
  evens = be[0] + be[2];
  odds = be[1] + be[3];

  /* trap zeros */
  lows = fabs( lows ) < 0.001 ? 0.001 : lows;
  odds = fabs( odds ) < 0.001 ? 0.001 : odds;

  energy = lows + highs;
  ratio = highs / lows;
  twist = evens / odds;

  ratio = fabs( ratio ) > 20 ? 20 : ratio;
  twist = fabs( twist ) > 20 ? 20 : twist;

  e[chunk][0] = energy;
  r[chunk][0] = ratio;
  t[chunk][0] = twist;
}
