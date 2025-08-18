// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>


#ifndef __R_BSP__
#define __R_BSP__


extern seg_t* curline;
extern side_t* sidedef;
extern line_t* linedef;
extern sector_t* frontsector;
extern sector_t* backsector;

extern int rw_x;
extern int rw_stopx;

extern boolean segtextured;

// false if the back side is the same plane
extern boolean markfloor;
extern boolean markceiling;

extern boolean skymap;

extern drawseg_t drawsegs[MAXDRAWSEGS];
extern drawseg_t* ds_p;

extern lighttable_t** hscalelight;
extern lighttable_t** vscalelight;
extern lighttable_t** dscalelight;


typedef void (*drawfunc_t)(int start, int stop);


// BSP?
void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);


void R_RenderBSPNode(int bspnum);


#endif
