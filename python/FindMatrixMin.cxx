int FindMatrixMin() {

  TFile file( "/sps/atlas/c/cgoudet/Calibration/ScaleResults/160503/DataOff_13TeV_25ns.root" );
  TMatrixD *matrix = (TMatrixD*) file.Get( "combinErr_alpha" );

  unsigned int minCol, minRow;
  double minVal;
  for ( unsigned int iRow = 0; iRow < matrix->GetNrows(); iRow++ ) {
    for ( unsigned int iCol = 0; iCol < matrix->GetNcols(); iCol++ ) {
      if ( !iRow && !iCol ) {
        minVal = (*matrix)(iRow, iCol);
        minCol = iCol;
        minRow = iRow;
        }
      else if ( (*matrix)(iRow, iCol) < minVal ) {
        minVal = (*matrix)(iRow, iCol);
        minCol = iCol;
        minRow = iRow;
      }
    }
  }

  cout << "minMatrix :" << endl;
  cout << "coordinates : " << minRow << " " << minCol << endl;
  cout << "min : " << minVal << endl;
  return 0;
}
