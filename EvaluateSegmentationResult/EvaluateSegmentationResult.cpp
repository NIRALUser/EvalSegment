#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageRegionIterator.h>
#include <itkPoint.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryThresholdImageFilter.h> 
#include <itkSubtractImageFilter.h>
#include <ANN/ANN.h>
#include "itkMinimumMaximumImageCalculator.h"




typedef itk::Image<unsigned char, 3>                      SegmentationType;
typedef SegmentationType::Pointer                         SegmentationPointer;
typedef itk::ImageFileReader< SegmentationType >          SegmentationReaderType;
typedef itk::ImageRegionIterator<SegmentationType>        IteratorType;
typedef itk::BinaryBallStructuringElement<char, 3>        StructuringType;
typedef itk::BinaryErodeImageFilter<SegmentationType, 
  SegmentationType, StructuringType>                      ErodeFilterType;
typedef itk::BinaryThresholdImageFilter< SegmentationType,
  SegmentationType>                                       ThreshFilterType;
typedef itk::SubtractImageFilter<SegmentationType, 
  SegmentationType, SegmentationType>                     SubFilterType;
typedef itk::Point<float,3>                               PointType;

typedef itk::MinimumMaximumImageCalculator< SegmentationType > MinMaxCalcType;

static int refPerc = 25;
// reference error for comparison => if a score of 90 = rater performance => refPerc 10

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cout << "Usage: " << argv[0] << " segmentation reference [-c voe rvd asd rmssd msd] [-o OutputName] [-r refPerfScore}" << std::endl;
    std::cout << "     -c    reference numbers, voe=Volume overlap error, rvd = relative absolute diffference percentage, asd = average surface distance, rmssd = avgSqrDistance, msd = maximalDistance" << std::endl;
    std::cout << "Output: resultFilename, referenceFilename,volumeSeg, volumeRef, volumeDif, volumeDifPerc, fabs( volumeDifPerc ),avgDistance, avgSqrDistance, maxDistance,tanimotoError,diceError" << std::endl;
    std::cout << "     -r refPerfScore reference error for comparison => if a score of 90 is equal to a rater performance then this should be 100-90=10" << std::endl;
    return -1;
  }

  // check if metrics should be scored according to given references
  bool calcScores = false;
  double refVOE=0, refRVD=0, refASD=0, refRMSSD=0, refMSD=0;
  for (int i=3; i<argc; i++) {
    if ( strcmp( argv[i], "-c" )==0 && i<(argc-1)) {
      refVOE = atof( argv[i+1] );
      refRVD = atof( argv[i+2] );
      refASD= atof( argv[i+3] );
      refRMSSD = atof( argv[i+4] );
      refMSD = atof( argv[i+5] );
      if (refVOE!=0 && refRVD!=0 && refASD!=0 && refRMSSD!=0 && refMSD!=0) calcScores = true;
      else std::cout << "Reference values have to be >0! Not calculating scores..." << std::endl;
      break;
    }
  }
  // reference performance
  for (int i=3; i<argc; i++) {
    if ( strcmp( argv[i], "-r" )==0 && i<(argc-1)) {
      refPerc = atoi( argv[i+1] );
      break;
    }
  }

  // save evaluation data with this name
  std::string evaluationName = "";
  for (int i=3; i<argc; i++) {
    if ( strcmp( argv[i], "-o" )==0 && i<(argc-1)) {
      evaluationName = argv[i+1];
      break;
    }
  }
  if (evaluationName == "") {
    evaluationName = "evaluation.txt";
  }

  std::string scoreName = "";
  if (calcScores) {
    scoreName = "scoring-" + evaluationName;
  }
  
  // initialisation
  SegmentationReaderType::Pointer resultReader = SegmentationReaderType::New();
  SegmentationReaderType::Pointer validationReader = SegmentationReaderType::New();
  SegmentationPointer resultImage = resultReader->GetOutput();
  SegmentationPointer validationImage = validationReader->GetOutput();
  StructuringType structuringBall;
  structuringBall.SetRadius( 1 );
  structuringBall.CreateStructuringElement();
  ThreshFilterType::Pointer threshFilter1 = ThreshFilterType::New();
  threshFilter1->SetLowerThreshold(1);
  threshFilter1->SetUpperThreshold(255);
  threshFilter1->SetOutsideValue (0);
  threshFilter1->SetInsideValue (1);
  ErodeFilterType::Pointer erodeFilter1 = ErodeFilterType::New();
  erodeFilter1->SetInput( threshFilter1->GetOutput() );
  erodeFilter1->SetKernel( structuringBall );
  erodeFilter1->SetErodeValue( 1 );
  SubFilterType::Pointer subFilter1 = SubFilterType::New();
  subFilter1->SetInput2( erodeFilter1->GetOutput() );
  ThreshFilterType::Pointer threshFilter2 = ThreshFilterType::New();
  threshFilter2->SetLowerThreshold(1);
  threshFilter2->SetUpperThreshold(255);
  threshFilter2->SetOutsideValue (0);
  threshFilter2->SetInsideValue (1);
  ErodeFilterType::Pointer erodeFilter2 = ErodeFilterType::New();
  erodeFilter2->SetInput( threshFilter2->GetOutput() );
  erodeFilter2->SetKernel( structuringBall );
  erodeFilter2->SetErodeValue( 1 );
  SubFilterType::Pointer subFilter2 = SubFilterType::New();
  subFilter2->SetInput2( erodeFilter2->GetOutput() );
  

  char resultFilename[512];
  sprintf( resultFilename, "%s", argv[1] );
  char referenceFilename[512];
  sprintf( referenceFilename, "%s", argv[2] );

  // read result image:
  try {
    resultReader->SetFileName( resultFilename );
    resultReader->Update();
  }
  catch(...) {
    printf( "Could not load image %s\n", resultFilename );
    return -2;
  }
  // read validation image:
  try {
    validationReader->SetFileName( referenceFilename );
    validationReader->Update();
  }
  catch(...) {
    printf( "Could not load image %s\n", referenceFilename );
    return -2;
  }
  // check if images have the same size
  SegmentationType::RegionType resRegion = resultImage->GetLargestPossibleRegion();
  SegmentationType::RegionType valRegion = validationImage->GetLargestPossibleRegion();
  if (resRegion.GetSize() != valRegion.GetSize())
  {
    printf( "Image sizes for %s and %s are different!\n", resultFilename, referenceFilename );
    return -3;
  }

  SegmentationType::SpacingType valSpacing = validationImage->GetSpacing();
  double volumeFactor = 0.001*valSpacing[0]*valSpacing[1]*valSpacing[2];
  SegmentationType::SpacingType resSpacing = resultImage->GetSpacing();
  if (resSpacing[0]!=valSpacing[0] || resSpacing[1]!=valSpacing[1] || resSpacing[2]!=valSpacing[2]) 
  {
    std::cerr << "WARNING: Spacing of segmentation different from reference! Sum "
            << fabs(resSpacing[0] - valSpacing[0]) + fabs(resSpacing[1] - valSpacing[1]) + fabs(resSpacing[2] - valSpacing[2])
            << std::endl;

  }
  PointType pnt;

  // compute border pixels and init kd-tree for image 1
  threshFilter1->SetInput( resultImage );
  subFilter1->SetInput1( threshFilter1->GetOutput() );
  subFilter1->UpdateLargestPossibleRegion();
  SegmentationPointer borderImg1 = subFilter1->GetOutput();

  // check if file is empty, if so create bogus single voxel at (1,1,1)
  MinMaxCalcType::Pointer minmaxCalc = MinMaxCalcType::New();
  minmaxCalc->SetImage(borderImg1);
  minmaxCalc->Compute();
  double max = minmaxCalc->GetMaximum();
  //std::cerr << "Max:"  << max << std::endl;
  if (max == 0) {
    SegmentationType::IndexType index;
    index[0] = index[1] = index[2] = 1;
    borderImg1->SetPixel(index,1);
    std::cerr << "WARNING: Empty border image in result, creating bogus segmentation for evaluation" << std::endl;
  }

  IteratorType it1( borderImg1, borderImg1->GetLargestPossibleRegion() );
  unsigned int numBorderPts1 = 0;
  for ( it1.GoToBegin(); !it1.IsAtEnd(); ++it1 ) {
    if (it1.Get() != 0) numBorderPts1++;
  }
  ANNpointArray borderPts1 = annAllocPts( numBorderPts1, 3 );
  numBorderPts1 = 0;
  for ( it1.GoToBegin(); !it1.IsAtEnd(); ++it1 ) {
    if (it1.Get() != 0) {
      validationImage->TransformIndexToPhysicalPoint( it1.GetIndex(), pnt );
      for (int d=0; d<3; d++) borderPts1[numBorderPts1][d] = pnt[d];
      numBorderPts1++;
    }
  }
  ANNkd_tree *borderTree1 = new ANNkd_tree( borderPts1, numBorderPts1, 3 );

  // compute border pixels and init kd-tree for image 2
  threshFilter2->SetInput( validationImage );
  subFilter2->SetInput1( threshFilter2->GetOutput() );
  subFilter2->UpdateLargestPossibleRegion();
  SegmentationPointer borderImg2 = subFilter2->GetOutput();

  minmaxCalc->SetImage(borderImg2);
  minmaxCalc->Compute();
  max = minmaxCalc->GetMaximum();
  //std::cerr << "Max:"  << max << std::endl;
  if (max == 0) {
    SegmentationType::IndexType index;
    index[0] = index[1] = index[2] = 1;
    borderImg2->SetPixel(index,1);
    std::cerr << "WARNING: Empty border image in reference, creating bogus segmentation for evaluation" << std::endl;
  }

  IteratorType it2( borderImg2, borderImg2->GetLargestPossibleRegion() );
  unsigned int numBorderPts2 = 0;
  for ( it2.GoToBegin(); !it2.IsAtEnd(); ++it2 ) {
    if (it2.Get() != 0) numBorderPts2++;
  }
  ANNpointArray borderPts2 = annAllocPts( numBorderPts2, 3 );
  numBorderPts2 = 0;
  for ( it2.GoToBegin(); !it2.IsAtEnd(); ++it2 ) {
    if (it2.Get() != 0) {
      validationImage->TransformIndexToPhysicalPoint( it2.GetIndex(), pnt );
      for (int d=0; d<3; d++) borderPts2[numBorderPts2][d] = pnt[d];
      numBorderPts2++;
    }
  }
  ANNkd_tree *borderTree2 = new ANNkd_tree( borderPts2, numBorderPts2, 3 );
  
  // calculate surface distance measures
  double avgDistance = 0;
  double avgSqrDistance = 0;
  double maxDistance = 0;
  ANNidxArray  nnIdx = new ANNidx[1];
  ANNdistArray dists = new ANNdist[1];
  
  for(unsigned int idx1=0; idx1<numBorderPts1; idx1++) {
    borderTree2->annkSearch( borderPts1[idx1], 1, nnIdx, dists);
    avgSqrDistance += dists[0];
    double d = sqrt( dists[0] );
    avgDistance += d;
    if (d>maxDistance) maxDistance = d;
  }

  for(unsigned int idx2=0; idx2<numBorderPts2; idx2++) {
    borderTree1->annkSearch( borderPts2[idx2], 1, nnIdx, dists);
    avgSqrDistance += dists[0];
    double d = sqrt( dists[0] );
    avgDistance += d;
    if (d>maxDistance) maxDistance = d;
  }

  double numBorderPts = numBorderPts1 + numBorderPts2;
  avgDistance /= numBorderPts;
  avgSqrDistance /= numBorderPts;
  avgSqrDistance = sqrt( avgSqrDistance );
  char resultBuffer[1024];

  // Tanimoto overlap metric
  unsigned long volume1=0, volume2=0, intersection=0;
  IteratorType resIt( resultImage, resRegion ), valIt( validationImage, valRegion );
  for ( resIt.GoToBegin(), valIt.GoToBegin(); !resIt.IsAtEnd(); ++resIt, ++valIt ) {
    if (resIt.Get()!=0) {
      volume1++;
      if (valIt.Get()!=0) {
        volume2++;
        intersection++;
      }
    }
    else {
      if (valIt.Get()!=0) volume2++;
    }
  }
  double tanimotoVal = 100.0 * (double)(intersection) / ((double)(volume1+volume2-intersection));
  double diceVal = 100.0 * (double) (intersection) / (((double) (volume1 + volume2)) / 2.0);
  double tanimotoError = 100.0 - tanimotoVal;
  double diceError = 100.0 - diceVal;
  double volumeSeg = (double)volume1 * volumeFactor;
  double volumeRef = (double)volume2 * volumeFactor;
  double volumeDif = volumeSeg - volumeRef;
  double volumeDifPerc = 100.0 * volumeDif / volumeRef;
  
  sprintf( resultBuffer, "%s; %s; %.4f; %.4f; %.4f; %.4f; %.4f; %.4f; %.4f; %.4f; %.2f; %.2f\n",
	   resultFilename, referenceFilename,
	   volumeSeg, volumeRef, volumeDif, volumeDifPerc, fabs( volumeDifPerc ),
	   avgDistance, avgSqrDistance, maxDistance,
	   tanimotoError,diceError );

  printf( resultBuffer );

  // append info to specified text file:
  FILE *file = fopen( evaluationName.c_str(), "a" );
  fputs( resultBuffer, file );
  fclose( file );

  // check if score file has to be written
  if (calcScores) {
    double scoreVOE = 100 - refPerc *( tanimotoError / refVOE );
    if (scoreVOE < 0) scoreVOE = 0;
    double scoreRVD = 100 - refPerc * ( fabs( volumeDifPerc ) / refRVD );
    if (scoreRVD < 0) scoreRVD = 0;
    double scoreASD = 100 - refPerc * ( avgDistance / refASD );
    if (scoreASD < 0) scoreASD = 0;
    double scoreRMSSD = 100 - refPerc * ( avgSqrDistance / refRMSSD );
    if (scoreRMSSD < 0) scoreRMSSD = 0;
    double scoreMSD = 100 - refPerc * ( maxDistance / refMSD );
    if (scoreMSD < 0) scoreMSD = 0;
    double totalScore = (scoreVOE+scoreRVD+scoreASD+scoreRMSSD+scoreMSD) / 5;
    printf( "  => Total Score = %.f\n", totalScore );
    sprintf( resultBuffer, "%s; %.f; %.f; %.f; %.f; %.f; %.f\n", 
      resultFilename, scoreVOE, scoreRVD, scoreASD, scoreRMSSD, scoreMSD, totalScore );
    file = fopen( scoreName.c_str(), "a" );
    fputs( resultBuffer, file );
    fclose( file );
  }

  // clean up
  annDeallocPts( borderPts1 );
  annDeallocPts( borderPts2 );
  delete borderTree1;
  delete borderTree2;
  delete[] nnIdx;
  delete[] dists;

  return 0;
}
