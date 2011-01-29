/*

Compare
Copyright (C) 1995 Jack Lynch, jlynch@english.upenn.edu.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

To receive a copy of the GNU General Public License, see
<http://www.gnu.org/copyleft/gpl.html> or write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA.
*/

/* Modfied by Kurt Rosenfeld 2004 */

float compare (char *s1, char *s2) {
  int i, j, k, x, y;
  float n = 0;
  
  for (i=0; s1[i]; i++) {
    x = i;
    for (j=0; s2[j]; j++) {
      y = j;
      k = 0;
      while ((s1[x] && s2[y]) && (s1[x] == s2[y])) {
		k++;
		n += (float)(k*k)/4;
		x++;
		y++;
      }
    }
  }
  n /= strlen (s1) * strlen (s2);
  return n;
}
