This program was written for use in the MICCAI Grand Challenge 2007.

Workshop website: http://mbi.dkfz-heidelberg.de/grand-challenge2007/

------------------------------------------------------------------------------

EvaluateSegmentationResult is a program to compare a 3D-segmentation with a 
reference. Both files have to be in ITK-readable format (e.g. MHD) with 8-bit 
pixel values, any value other than zero counts as object.
To compile, you need a recent ITK (downloadable from http://www.itk.org),
CMake (http://www.cmake.org) and the ANN-library 
(http://www.cs.umd.edu/~mount/ANN/).

First compile ANN (or download a precompiled version).
If you have not done so yet, download CMake and use it to compile ITK.
When configuring EvaluateSegmentationResult with CMake, set ANN_PATH to the
ANN include directory (for Windows precompile it's the main directory) where
the ANN subdirectory in located. 

After successful compilation, you can run the program with segmentation and
reference filename as parameters. It will output the results to the screen
and append them to evaluation.txt (or another file specified with the -o
option). The output format is:

SegmentationFileName; ReferenceFileName; VolumeOfSegmentation[ml]; 
VolumeOfReference[ml]; SignedVolumetricDifference[ml]; 
SignedRelativeVolumeDiffeence[%]; AbsoluteRelativeVolumeDifference[%];
AverageSymmetricSurfaceDistance[mm]; AverageSymmetricRMSSurfaceDistance[mm]; 
MaximumSurfaceDistance[mm]; VolumetricOverlapError[%]

The metrics used during the contest are:
- VolumetricOverlapError[%]
- AbsoluteRelativeVolumeDifference[%]
- AverageSymmetricSurfaceDistance[mm]
- AverageSymmetricRMSSurfaceDistance[mm]
- MaximumSurfaceDistance[mm]

The weighting factors to score these metrics are determined by the average results of a human second rater. For the liver, preliminary results are already posted on the workshop website in the Evaluation section. For the caudate, results will be posted soon or sent by mail to all participants.
Once results for the second rater are known, you can pass them to the evaluation program using the -c parameter: In addition to the comparison metrics, it will then also calculate the corresponding scores.


For any questions or comments contact Tobias Heimann (t.heimann@dkfz.de)

