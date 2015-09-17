#ifndef INVERTMATRIX_H
#define INVERTMATRIX_H

#include "TH2D.h"
#include "TMatrixD.h"

/**\brief obtain alpha_i from alpha_ij matrix
   \param combinMatrix TMatrixD of alpha_ij
   \param combinErrMatrix TMatrixD of uncertainty on alpha_ij
   \param outMatrx matrix filled with result alpha_i
   \param outErrMatrix matrix fille with uncertainty results

 */
void InvertMatrix( TMatrixD &combinMatrix, TMatrixD &combinErrMatrix, TMatrixD &outMatrix, TMatrixD &outErrMatrix, unsigned int inversionProcedure = 0 );

/**\brief return square root of |inVal| with the same sign as inVal
 */
double SignSquare( double inVal );

/**\brief compute uncertainty of C from C2 uncertainty
 */
double ErrC( double c, double errC2 );

#endif
