#ifndef INVERTMATRIX_H
#define INVERTMATRIX_H

#include "TH2D.h"
#include "TMatrixD.h"

namespace ChrisLib {
/**\brief obtain alpha_i from alpha_ij matrix
   \param combinMatrix TMatrixD of alpha_ij
   \param combinErrMatrix TMatrixD of uncertainty on alpha_ij
   \param outMatrx matrix filled with result alpha_i
   \param outErrMatrix matrix fille with uncertainty results
   \param inversionProcedure Mode d'inversion

   ### inversionProcedure
   - 0X : Inversion is performed analytically
   - 1X : Inversion is performed as likelihood maximization

   Formulas :
   - X0 : alpha_ij = ( alpha_i + alpha_j )/2
   - X1 : alpha_ij^2 = ( (alpha^2)_i^2 + alpha_j^2 ) /2
   - X2 : alpha_ij = sqrt( alpha_i^2 + alpha_j^2 ) /2
   - X3 and X4 : tests to deal with multi dimensional scales. Not validated.
*/
void InvertMatrix( TMatrixD &combinMatrix, TMatrixD &combinErrMatrix, TMatrixD &outMatrix, TMatrixD &outErrMatrix, unsigned int inversionProcedure = 0 );

/**\brief return square root of |inVal| with the same sign as inVal
 */
 double SignSquare( double inVal );

/**\brief compute uncertainty of C from C2 uncertainty
 */
 double ErrC( double c, double errC2 );

}
#endif
