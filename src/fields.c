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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"

#include "fields.h"
#include "utils.h"
#include "chemf.h"
#include "cells.h"


void allocateFieldGradient(systeminfo_t systeminfo,fieldgradientdata_t *fieldgradientdata)
{

        if (!gfields)
                return;

        MPI_Cart_shift(systeminfo.MPI_CART_COMM,0,1,&fieldgradientdata->bx0,&fieldgradientdata->bx1);
        MPI_Cart_shift(systeminfo.MPI_CART_COMM,1,1,&fieldgradientdata->by0,&fieldgradientdata->by1);
        MPI_Cart_shift(systeminfo.MPI_CART_COMM,2,1,&fieldgradientdata->bz0,&fieldgradientdata->bz1);

        /* allocate send buffers */
        if(fieldgradientdata->bx0!=MPI_PROC_NULL) fieldgradientdata->sendx0=(double*)calloc(gridSize.y*gridSize.z,sizeof(double));
        if(fieldgradientdata->bx1!=MPI_PROC_NULL) fieldgradientdata->sendx1=(double*)calloc(gridSize.y*gridSize.z,sizeof(double));
        if(fieldgradientdata->by0!=MPI_PROC_NULL) fieldgradientdata->sendy0=(double*)calloc(gridSize.x*gridSize.z,sizeof(double));
        if(fieldgradientdata->by1!=MPI_PROC_NULL) fieldgradientdata->sendy1=(double*)calloc(gridSize.x*gridSize.z,sizeof(double));
        if(fieldgradientdata->bz0!=MPI_PROC_NULL) fieldgradientdata->sendz0=(double*)calloc(gridSize.y*gridSize.x,sizeof(double));
        if(fieldgradientdata->bz1!=MPI_PROC_NULL) fieldgradientdata->sendz1=(double*)calloc(gridSize.y*gridSize.x,sizeof(double));

        /* allocate receive buffers */
        if(fieldgradientdata->bx0!=MPI_PROC_NULL) fieldgradientdata->recvx0=(double*)calloc(gridSize.y*gridSize.z,sizeof(double));
        if(fieldgradientdata->bx1!=MPI_PROC_NULL) fieldgradientdata->recvx1=(double*)calloc(gridSize.y*gridSize.z,sizeof(double));
        if(fieldgradientdata->by0!=MPI_PROC_NULL) fieldgradientdata->recvy0=(double*)calloc(gridSize.x*gridSize.z,sizeof(double));
        if(fieldgradientdata->by1!=MPI_PROC_NULL) fieldgradientdata->recvy1=(double*)calloc(gridSize.x*gridSize.z,sizeof(double));
        if(fieldgradientdata->bz0!=MPI_PROC_NULL) fieldgradientdata->recvz0=(double*)calloc(gridSize.y*gridSize.x,sizeof(double));
        if(fieldgradientdata->bz1!=MPI_PROC_NULL) fieldgradientdata->recvz1=(double*)calloc(gridSize.y*gridSize.x,sizeof(double));

        return;
}

void initFieldHaloExchange(systeminfo_t systeminfo, fieldgradientdata_t *fieldgradientdata, int chf)
{
        int i,j,k;
        if (!gfields)
                return;

        for(i=0; i<gridSize.x; i++)
                for(j=0; j<gridSize.y; j++)
                        for(k=0; k<gridSize.z; k++) {
                                double val;
                                val=fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*j+k];
                                if(i==0 && fieldgradientdata->bx0!=MPI_PROC_NULL) fieldgradientdata->sendx0[gridSize.z*j+k]=val;
                                if(i==gridSize.x-1 && fieldgradientdata->bx1!=MPI_PROC_NULL) fieldgradientdata->sendx1[gridSize.z*j+k]=val;
                                if(j==0 && fieldgradientdata->by0!=MPI_PROC_NULL) fieldgradientdata->sendy0[gridSize.z*i+k]=val;
                                if(j==gridSize.y-1 && fieldgradientdata->by1!=MPI_PROC_NULL) fieldgradientdata->sendy1[gridSize.z*i+k]=val;
                                if(k==0 && fieldgradientdata->bz0!=MPI_PROC_NULL) fieldgradientdata->sendz0[gridSize.y*i+j]=val;
                                if(k==gridSize.z-1 && fieldgradientdata->bz1!=MPI_PROC_NULL) fieldgradientdata->sendz1[gridSize.y*i+j]=val;
                        }

        if(fieldgradientdata->bx0!=MPI_PROC_NULL) {
                MPI_Isend(fieldgradientdata->sendx0,gridSize.y*gridSize.z,MPI_DOUBLE,fieldgradientdata->bx0,systeminfo.rank,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqsend)[0]);
                MPI_Irecv(fieldgradientdata->recvx0,gridSize.y*gridSize.z,MPI_DOUBLE,fieldgradientdata->bx0,systeminfo.size+fieldgradientdata->bx0,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqrecv)[0]);
        }
        if(fieldgradientdata->bx1!=MPI_PROC_NULL) {
                MPI_Isend(fieldgradientdata->sendx1,gridSize.y*gridSize.z,MPI_DOUBLE,fieldgradientdata->bx1,systeminfo.size+systeminfo.rank,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqsend)[1]);
                MPI_Irecv(fieldgradientdata->recvx1,gridSize.y*gridSize.z,MPI_DOUBLE,fieldgradientdata->bx1,fieldgradientdata->bx1,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqrecv)[1]);
        }
        if(fieldgradientdata->by0!=MPI_PROC_NULL) {
                MPI_Isend(fieldgradientdata->sendy0,gridSize.x*gridSize.z,MPI_DOUBLE,fieldgradientdata->by0,2*systeminfo.size+systeminfo.rank,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqsend)[2]);
                MPI_Irecv(fieldgradientdata->recvy0,gridSize.x*gridSize.z,MPI_DOUBLE,fieldgradientdata->by0,3*systeminfo.size+fieldgradientdata->by0,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqrecv)[2]);
        }
        if(fieldgradientdata->by1!=MPI_PROC_NULL) {
                MPI_Isend(fieldgradientdata->sendy1,gridSize.x*gridSize.z,MPI_DOUBLE,fieldgradientdata->by1,3*systeminfo.size+systeminfo.rank,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqsend)[3]);
                MPI_Irecv(fieldgradientdata->recvy1,gridSize.x*gridSize.z,MPI_DOUBLE,fieldgradientdata->by1,2*systeminfo.size+fieldgradientdata->by1,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqrecv)[3]);
        }
        if(fieldgradientdata->bz0!=MPI_PROC_NULL) {
                MPI_Isend(fieldgradientdata->sendz0,gridSize.y*gridSize.x,MPI_DOUBLE,fieldgradientdata->bz0,4*systeminfo.size+systeminfo.rank,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqsend)[4]);
                MPI_Irecv(fieldgradientdata->recvz0,gridSize.y*gridSize.x,MPI_DOUBLE,fieldgradientdata->bz0,5*systeminfo.size+fieldgradientdata->bz0,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqrecv)[4]);
        }
        if(fieldgradientdata->bz1!=MPI_PROC_NULL) {
                MPI_Isend(fieldgradientdata->sendz1,gridSize.y*gridSize.x,MPI_DOUBLE,fieldgradientdata->bz1,5*systeminfo.size+systeminfo.rank,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqsend)[5]);
                MPI_Irecv(fieldgradientdata->recvz1,gridSize.y*gridSize.x,MPI_DOUBLE,fieldgradientdata->bz1,4*systeminfo.size+fieldgradientdata->bz1,systeminfo.MPI_CART_COMM,&(fieldgradientdata->reqrecv)[5]);
        }

        return;
}

void waitFieldHaloExchange(fieldgradientdata_t *fieldgradientdata)
{
        if (!gfields)
                return;
        MPI_Status status;
        if(fieldgradientdata->bx0!=MPI_PROC_NULL) {
                if (MPI_Wait(&fieldgradientdata->reqsend[0], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
                if (MPI_Wait(&fieldgradientdata->reqrecv[0], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
        }
        if(fieldgradientdata->bx1!=MPI_PROC_NULL) {
                if (MPI_Wait(&fieldgradientdata->reqsend[1], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
                if (MPI_Wait(&fieldgradientdata->reqrecv[1], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
        }
        if(fieldgradientdata->by0!=MPI_PROC_NULL) {
                if (MPI_Wait(&fieldgradientdata->reqsend[2], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
                if (MPI_Wait(&fieldgradientdata->reqrecv[2], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
        }
        if(fieldgradientdata->by1!=MPI_PROC_NULL) {
                if (MPI_Wait(&fieldgradientdata->reqsend[3], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
                if (MPI_Wait(&fieldgradientdata->reqrecv[3], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
        }
        if(fieldgradientdata->bz0!=MPI_PROC_NULL) {
                if (MPI_Wait(&fieldgradientdata->reqsend[4], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
                if (MPI_Wait(&fieldgradientdata->reqrecv[4], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
        }
        if(fieldgradientdata->bz1!=MPI_PROC_NULL) {
                if (MPI_Wait(&fieldgradientdata->reqsend[5], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
                if (MPI_Wait(&fieldgradientdata->reqrecv[5], &status) != MPI_SUCCESS)
                        stopRun(103, "fieldgradientdata->reqsend", __FILE__, __LINE__);
        }

        return;
}


void computeFieldGradient(fieldgradientdata_t *fieldgradientdata, int chf)
{
        int i,j,k;
        if (!gfields)
                return;
        /* compute internal data */
        for(i=0; i<gridSize.x; i++)
                for(j=0; j<gridSize.y; j++)
                        for(k=0; k<gridSize.z; k++) {
                                /* x coord gradient */
                                if(i!=0 && i!=gridSize.x-1) {
                                        gradAddr[chf][3*gridSize.z*gridSize.y*i+3*gridSize.z*j+3*k+0]=
                                                (fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*(i+1)+gridSize.z*j+k]
                                                 -fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*(i-1)+gridSize.z*j+k])/gridResolution;

                                }
                                /* y coord gradient */
                                if(j!=0 && j!=gridSize.y-1) {
                                        gradAddr[chf][3*gridSize.z*gridSize.y*i+3*gridSize.z*j+3*k+1]=
                                                (fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*(j+1)+k]
                                                 -fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*(j-1)+k])/gridResolution;
                                }
                                /* z coord gradient */
                                if(k!=0 && k!=gridSize.z-1) {
                                        gradAddr[chf][3*gridSize.z*gridSize.y*i+3*gridSize.z*j+3*k+2]=
                                                (fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*j+k+1]
                                                 -fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*j+k-1])/gridResolution;

                                }
                        }
        /* wait for boundary data to arrive */
        waitFieldHaloExchange(fieldgradientdata);
        /* update with boundary data */
        for(j=0; j<gridSize.y; j++)
                for(k=0; k<gridSize.z; k++) {
                        double x0,x1;
                        if(fieldgradientdata->bx0!=MPI_PROC_NULL) x0=fieldgradientdata->recvx0[gridSize.z*j+k];
                        else x0=fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*0+gridSize.z*j+k];
                        if(fieldgradientdata->bx1!=MPI_PROC_NULL) x1=fieldgradientdata->recvx1[gridSize.z*j+k];
                        else x1=fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*(gridSize.x-1)+gridSize.z*j+k];
                        gradAddr[chf][3*gridSize.z*gridSize.y*0+3*gridSize.z*j+3*k+0]=
                                (fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*1+gridSize.z*j+k]
                                 - x0)/gridResolution;
                        gradAddr[chf][3*gridSize.z*gridSize.y*(gridSize.x-1)+3*gridSize.z*j+3*k+0]=
                                (x1
                                 - fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*(gridSize.x-2)+gridSize.z*j+k])/gridResolution;
                }
        for(i=0; i<gridSize.x; i++)
                for(k=0; k<gridSize.z; k++) {
                        double y0,y1;
                        if(fieldgradientdata->by0!=MPI_PROC_NULL) y0=fieldgradientdata->recvy0[gridSize.z*i+k];
                        else y0=fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*0+k];
                        if(fieldgradientdata->by1!=MPI_PROC_NULL) y1=fieldgradientdata->recvy1[gridSize.z*i+k];
                        else y1=fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*(gridSize.y-1)+k];
                        gradAddr[chf][3*gridSize.z*gridSize.y*i+3*gridSize.z*0+3*k+1]=
                                (fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*1+k]
                                 - y0)/gridResolution;
                        gradAddr[chf][3*gridSize.z*gridSize.y*i+3*gridSize.z*(gridSize.y-1)+3*k+1]=
                                (y1
                                 - fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*(gridSize.y-2)+k])/gridResolution;
                }
        for(i=0; i<gridSize.x; i++)
                for(j=0; j<gridSize.y; j++) {
                        double z0,z1;
                        if(fieldgradientdata->bz0!=MPI_PROC_NULL) z0=fieldgradientdata->recvz0[gridSize.y*i+j];
                        else z0=fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*j+0];
                        if(fieldgradientdata->bz1!=MPI_PROC_NULL) z1=fieldgradientdata->recvz1[gridSize.y*i+j];
                        else z1=fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*j+(gridSize.z-1)];
                        gradAddr[chf][3*gridSize.z*gridSize.y*i+3*gridSize.z*j+3*0+2]=
                                (fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*j+1]
                                 - z0)/gridResolution;
                        gradAddr[chf][3*gridSize.z*gridSize.y*i+3*gridSize.z*j+3*(gridSize.z-1)+2]=
                                (z1
                                 - fieldAddr[NGLOB+chf][gridSize.z*gridSize.y*i+gridSize.z*j+(gridSize.z-2)])/gridResolution;
                }

        if(fieldgradientdata->bx0!=MPI_PROC_NULL) {
                free(fieldgradientdata->sendx0);
                free(fieldgradientdata->recvx0);
        }
        if(fieldgradientdata->bx1!=MPI_PROC_NULL) {
                free(fieldgradientdata->sendx1);
                free(fieldgradientdata->recvx1);
        }
        if(fieldgradientdata->by0!=MPI_PROC_NULL) {
                free(fieldgradientdata->sendy0);
                free(fieldgradientdata->recvy0);
        }
        if(fieldgradientdata->by1!=MPI_PROC_NULL) {
                free(fieldgradientdata->sendy1);
                free(fieldgradientdata->recvy1);
        }
        if(fieldgradientdata->bz0!=MPI_PROC_NULL) {
                free(fieldgradientdata->sendz0);
                free(fieldgradientdata->recvz0);
        }
        if(fieldgradientdata->bz1!=MPI_PROC_NULL) {
                free(fieldgradientdata->sendz1);
                free(fieldgradientdata->recvz1);
        }

        return;
}

void fieldGradient(systeminfo_t systeminfo)
{
        fieldgradientdata_t fieldgradientdata;
        int chf;
        if (!gfields)
                return;
        allocateFieldGradient(systeminfo,&fieldgradientdata);
        for(chf=0; chf<NCHEM; chf++) {
                if(chf==OXYG-NGLOB && !oxygen) continue;
                if(chf==GLUC-NGLOB && !glucose) continue;
                if(chf==HYDR-NGLOB && !hydrogenIon) continue;
                initFieldHaloExchange(systeminfo,&fieldgradientdata,chf);
                //waitFieldHaloExchange();
                computeFieldGradient(&fieldgradientdata,chf);

        }
        return;
}
