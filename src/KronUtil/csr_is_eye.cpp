#include "util.h"
bool csr_is_eye(const PsimagLite::CrsMatrix<double>& a)
{
   // ----------------------------------------------------
   // check whether a sparse matrix is the identity matrix
   // ----------------------------------------------------
        
   const int nrow_A = a.row();
   const int ncol_A = a.col();

   bool is_eye = (nrow_A == ncol_A);

   if (!is_eye) { return( false ); };

   if ((nrow_A <= 0) || (ncol_A <= 0)) {
      return( false );
      };

   for(int ia=0; ia < nrow_A; ia++) {
     int istart = a.getRowPtr(ia);
     int iend = a.getRowPtr(ia + 1);

     bool has_diagonal  = false;;
     for(int k=istart; k < iend; k++) {
        int ja = a.getCol(k);
        double aij = a.getValue(k);
        double eij = (ia == ja) ? 1 : 0;
        if (ia == ja) { has_diagonal = true; };

        is_eye = (aij == eij);
        if (!is_eye) { return( false ); };
       };
     if (!has_diagonal) { return( false ); };

    };

   return( true );
}
 
