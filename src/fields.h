/* **************************************************************************
 * This file is part of Timothy
 *
 * Copyright (c) 2014/15 Maciej Cytowski
 * Copyright (c) 2014/15 ICM, University of Warsaw, Poland
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * *************************************************************************/

/*! \file fields.h
 *  \brief contains variables and arrays for global fields
 */

 typedef struct fieldgradientdata_t {
   int bx0,bx1;
   int by0,by1;
   int bz0,bz1;
   double *sendx0;
   double *sendx1;
   double *sendy0;
   double *sendy1;
   double *sendz0;
   double *sendz1;
   double *recvx0;
   double *recvx1;
   double *recvy0;
   double *recvy1;
   double *recvz0;
   double *recvz1;
   MPI_Request reqsend[6];
   MPI_Request reqrecv[6];
 } fieldgradientdata_t;

#define NFIELDS 6 //5
#define NIF 2 /* number of fields that need to be interpolated from discrete data */
#define NGLOB 3
#define NCHEM 3
#define SCALAR_FIELD 1
#define VECTOR_FIELD 2

void allocatefields(systeminfo_t systeminfo,settings_t settings,grid_t grid,environment_t **environment,solverdata_t *solverdata);
void initfields(systeminfo_t systeminfo,settings_t settings,grid_t grid,environment_t **environment);
void fieldsInit();
void fieldsSolve(settings_t settings,cellsinfo_t *cellsinfo);
void allocateFieldGradient(systeminfo_t systeminfo,fieldgradientdata_t *fieldgradientdata);
void initFieldHaloExchange(systeminfo_t systeminfo,fieldgradientdata_t *fieldgradientdata,int chf);
void computeFieldGradient(fieldgradientdata_t *fieldgradientdata,int chf);
void fieldGradient();


int nfields;
int nif;
int nglob;
int nchem;

int gfields;
int oxygen;
int glucose;
int hydrogenIon;
int temperature;

/* grid data */
int64_t gridI,gridJ,gridK;
int643dv_t gridSize;
int643dv_t *gridStartIdx,*gridEndIdx;
double3dv_t *gridBuffer;
#define grid(i,j,k) (gridBuffer[gridSize.y*gridSize.z*i+gridSize.z*j+k])
double gridResolution;
double3dv_t lowerGridCorner,upperGridCorner;
double3dv_t globalGridSize;
double boxVolume;

/* field data */
char fieldName[NFIELDS][128];
double *fieldAddr[NFIELDS];
double *gradAddr[NCHEM];


int fieldNumberOfCriticalLevels[NFIELDS]; /* number of critical levels for each field */
//int *fieldNumberOfCriticalLevels; /* number of critical levels for each field */

double fieldCriticalLevel1[NFIELDS]; /* critical level 1 - for cells in G1 phase */
double fieldCriticalLevel2[NFIELDS]; /* critical level 2 - for cells in S, G2, M phases */
//double *fieldCriticalLevel1; /* critical level 1 - for cells in G1 phase */
//double *fieldCriticalLevel2; /* critical level 2 - for cells in S, G2, M phases */

double fieldMin[NFIELDS];
double fieldMax[NFIELDS];
//double *fieldMin;
//double *fieldMax;

double *tissueField;
double *vesselField;
double *tempField;
double *chemField[NCHEM];
double *chemGradient[NCHEM];
//double **chemField;
//double **chemGradient;
double **fieldsPatchesCommBuff;
double **fieldsPatches;

double *haloSX0,*haloSX1,*haloSY0,*haloSY1,*haloSZ0,*haloSZ1;
double *haloRX0,*haloRX1,*haloRY0,*haloRY1,*haloRZ0,*haloRZ1;
int rX0,rX1,rY0,rY1,rZ0,rZ1;

MPI_Request reqFGSend[6];
MPI_Request reqFGRecv[6];


#define DENS 0
#define BVES 1
#define TEMP 2
#define OXYG 3
#define GLUC 4
#define HYDR 5
