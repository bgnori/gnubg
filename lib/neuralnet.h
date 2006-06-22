/*
 * neuralnet.h
 *
 * by Gary Wong, 1998
 * $Id: neuralnet.h,v 1.13 2006/06/22 22:51:00 Superfly_Jon Exp $
 */

#ifndef _NEURALNET_H_
#define _NEURALNET_H_

#include <stdio.h>

#define SIGMOID_BAUR 0
#define SIGMOID_JTH 0

#if SIGMOID_BAUR
#define sigmoid sigmoid_baur
#elif SIGMOID_JTH
#define sigmoid sigmoid_jth
#else
extern /*inline*/ float sigmoid_original(float const xin);
#define sigmoid sigmoid_original
#endif

typedef struct _neuralnet {
    int cInput, cHidden, cOutput, nTrained, fDirect;
    float rBetaHidden, rBetaOutput, *arHiddenWeight, *arOutputWeight,
	*arHiddenThreshold, *arOutputThreshold;
    float *savedBase, *savedIBase;
} neuralnet;

extern int NeuralNetCreate( neuralnet *pnn, int cInput, int cHidden,
			    int cOutput, float rBetaHidden,
			    float rBetaOutput );

extern void *NeuralNetCreateDirect( neuralnet *pnn, void *p );

extern int NeuralNetDestroy( neuralnet *pnn );

typedef enum  {
  NNEVAL_NONE,
  NNEVAL_SAVE,
  NNEVAL_FROMBASE
} NNEvalType;

extern int (*NeuralNetEvaluateFn)( neuralnet *pnn, float arInput[],
			      float arOutput[], NNEvalType t);

extern int NeuralNetEvaluate( neuralnet *pnn, float arInput[],
			      float arOutput[], NNEvalType t);
extern int NeuralNetEvaluate128( neuralnet *pnn, float arInput[],
			      float arOutput[], NNEvalType t);
extern int NeuralNetDifferentiate( neuralnet *pnn, float arInput[],
				   float arOutput[], float arDerivative[] );
extern int NeuralNetTrain( neuralnet *pnn, float arInput[], float arOutput[],
			   float arDesired[], float rAlpha );
extern int NeuralNetResize( neuralnet *pnn, int cInput, int cHidden,
			    int cOutput );

extern int NeuralNetLoad( neuralnet *pnn, FILE *pf );
extern int NeuralNetLoadBinary( neuralnet *pnn, FILE *pf );
extern int NeuralNetSave( neuralnet *pnn, FILE *pf );
extern int NeuralNetSaveBinary( neuralnet *pnn, FILE *pf );

extern void 
ComputeSigTable (void);

extern int SSE_Supported();

#endif
