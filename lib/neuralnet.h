/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 or later of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * neuralnet.h
 *
 * by Gary Wong, 1998
 * $Id: neuralnet.h,v 1.19 2007/11/17 10:31:34 c_anthon Exp $
 */

#ifndef _NEURALNET_H_
#define _NEURALNET_H_
#include "common.h"
#include <stdio.h>
typedef struct _neuralnet {
	unsigned int cInput;
	unsigned int cHidden;
	unsigned int cOutput;
	unsigned int fDirect;
	int nTrained;
	float rBetaHidden;
	float rBetaOutput;
	float *arHiddenWeight;
	float *arOutputWeight;
	float *arHiddenThreshold;
	float *arOutputThreshold;
} neuralnet;

typedef enum {
	NNEVAL_NONE,
	NNEVAL_SAVE,
	NNEVAL_FROMBASE
} NNEvalType;

typedef enum {
	NNSTATE_NONE = -1,
	NNSTATE_INCREMENTAL,
	NNSTATE_DONE
} NNStateType;

typedef struct _NNState {
	NNStateType state;
	float *savedBase;
	float *savedIBase;
} NNState;

extern int (*NeuralNetEvaluateFn) (const neuralnet * pnn, float arInput[], float arOutput[], NNState * pnState);

extern float sigmoid(float const xin);
extern int NeuralNetCreate(neuralnet * pnn, unsigned int cInput, unsigned int cHidden, unsigned int cOutput, float rBetaHidden, float rBetaOutput);
extern void NeuralNetDestroy(neuralnet * pnn);
extern int NeuralNetEvaluate(const neuralnet * pnn, float arInput[], float arOutput[], NNState * pnState);
extern int NeuralNetEvaluate128(const neuralnet * pnn, float arInput[], float arOutput[], NNState * pnState);
extern int NeuralNetResize(neuralnet * pnn, unsigned int cInput, unsigned int cHidden, unsigned int cOutput);
extern int NeuralNetLoad(neuralnet * pnn, FILE * pf);
extern int NeuralNetLoadBinary(neuralnet * pnn, FILE * pf);
extern int NeuralNetSaveBinary(const neuralnet * pnn, FILE * pf);
extern int SSE_Supported(void);
#endif
