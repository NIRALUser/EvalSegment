#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>

// number of lines for scoring (both left/right)
#define NUM_INPUTS 48
// assuming left and right files follow in succession
// number of metrics in each file (without the total)
#define NUM_METRICS 5

#define NUM_REPS 20

static char datasetName[NUM_INPUTS/2][100] = 
  {"UNC Ped 10",   "UNC Ped 14",   "UNC Ped 15",   "UNC Ped 19",   "UNC Ped 30",   "UNC Eld 01",
   "UNC Eld 12",   "UNC Eld 13",  "UNC Eld 20",   "UNC Eld 26",  "BWH PNL 16",   "BWH PNL 17",
   "BWH PNL 18",   "BWH PNL 19",   "BWH PNL 20",   "BWH PNL 21",   "BWH PNL 22",   "BWH PNL 23",
   "BWH PNL 24",   "BWH PNL 25",   "BWH PNL 26",   "BWH PNL 27",  "BWH PNL 28",   "BWH PNL 29"};
static char plotName[NUM_INPUTS/2][100] = 
  {"Ped1",   "Ped2",   "Ped3",   "Ped4",   "Ped5",   "Eld1",
   "Eld2",   "Eld3",  "Eld4",   "Eld5",  "BWH16",   "BWH17",
   "BWH18",   "BWH19",   "BWH20",   "BWH21",   "BWH22",   "BWH23",
   "BWH24",   "BWH25",   "BWH26",   "BWH27",  "BWH28",   "BWH29"};

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf( "Usage: %s metricFile scoreFile [-o OutputName] \n", argv[0] );
    return -1;
  }

  // --------------------------------------------------------------------------------------------
  // Load all metrics
  // --------------------------------------------------------------------------------------------

  char line[1000];

  // store all metrics and average here:
  float metricVal[NUM_INPUTS+4][NUM_METRICS];
  float metricStdev[NUM_METRICS][4];

  for (int i=0; i<(NUM_INPUTS+4); i++) for (int j=0; j<NUM_METRICS; j++) metricVal[i][j] = 0.0;
  for (int cat = 0; cat < 4; cat++) {
    for (int j=0; j<NUM_METRICS; j++) {
      metricStdev[j][cat] = 0;
    }
  }

  FILE *metricFile = fopen( argv[1], "r" );
  if (!metricFile) {
    printf( "ERROR: Could not open %s\n", argv[1] );
    return -1;
  }
  char segFilename[1024], refFilename[1024], username[1024];
  float volumeSeg, volumeRef, volumeDif, volumeDifPerc, volumeDifPercAbs;
  float avgDistance, rmsDistance, maxDistance, tanimotoError;
  
  float allVolumeSeg[NUM_INPUTS], allVolumeRef[NUM_INPUTS];
  

  for (int i=0; i<NUM_INPUTS; i++) {
    fgets(line,1000,metricFile);
    int numScan = sscanf(line, " %s %s %f; %f; %f; %f; %f; %f; %f; %f; %f ", 
			 segFilename, refFilename,
      &volumeSeg, &volumeRef, &volumeDif, &volumeDifPerc, &volumeDifPercAbs,
	    &avgDistance, &rmsDistance, &maxDistance, &tanimotoError );
    allVolumeSeg[i] = volumeSeg;
    allVolumeRef[i] = volumeRef;
    metricVal[i][0] = tanimotoError;    metricVal[NUM_INPUTS][0] += tanimotoError;
    metricVal[i][1] = volumeDifPerc;    metricVal[NUM_INPUTS][1] += volumeDifPerc;
    metricVal[i][2] = avgDistance;      metricVal[NUM_INPUTS][2] += avgDistance;
    metricVal[i][3] = rmsDistance;      metricVal[NUM_INPUTS][3] += rmsDistance;
    metricVal[i][4] = maxDistance;      metricVal[NUM_INPUTS][4] += maxDistance;
    //std::cout << metricVal[i][0] << std::endl;
    if (i < 10) {
      metricVal[NUM_INPUTS+1][0] += tanimotoError;
      metricVal[NUM_INPUTS+1][1] += volumeDifPerc;
      metricVal[NUM_INPUTS+1][2] += avgDistance;
      metricVal[NUM_INPUTS+1][3] += rmsDistance;
      metricVal[NUM_INPUTS+1][4] += maxDistance;
    }
    if (i >= 10 && i < 20) {
      metricVal[NUM_INPUTS+2][0] += tanimotoError;
      metricVal[NUM_INPUTS+2][1] += volumeDifPerc;
      metricVal[NUM_INPUTS+2][2] += avgDistance;
      metricVal[NUM_INPUTS+2][3] += rmsDistance;
      metricVal[NUM_INPUTS+2][4] += maxDistance;
    }
    if (i >= 20 && i < NUM_INPUTS) {
      metricVal[NUM_INPUTS+3][0] += tanimotoError;
      metricVal[NUM_INPUTS+3][1] += volumeDifPerc;
      metricVal[NUM_INPUTS+3][2] += avgDistance;
      metricVal[NUM_INPUTS+3][3] += rmsDistance;
      metricVal[NUM_INPUTS+3][4] += maxDistance;
    }
  }
  // calculate average
  for (int j=0; j<NUM_METRICS; j++) metricVal[NUM_INPUTS][j] /=  NUM_INPUTS;
  for (int j=0; j<NUM_METRICS; j++) metricVal[NUM_INPUTS+1][j] /=  10;
  for (int j=0; j<NUM_METRICS; j++) metricVal[NUM_INPUTS+2][j] /=  10;
  for (int j=0; j<NUM_METRICS; j++) metricVal[NUM_INPUTS+3][j] /=  28;


  //calculate standard devation for metrics
  for (int i=0; i<NUM_INPUTS; i++) {
    for (int j=0; j<NUM_METRICS; j++) {
      metricStdev[j][0] = metricStdev[j][0] + ( metricVal[i][j] - metricVal[NUM_INPUTS+0][j] ) * ( metricVal[i][j] - metricVal[NUM_INPUTS+0][j] );
      
      if (i < 10) {
	metricStdev[j][1] = metricStdev[j][1] + ( metricVal[i][j] - metricVal[NUM_INPUTS+1][j] ) * ( metricVal[i][j] - metricVal[NUM_INPUTS+1][j] );
      }
      if (i >= 10 && i < 20) {
	metricStdev[j][2] = metricStdev[j][2] + ( metricVal[i][j] - metricVal[NUM_INPUTS+2][j] ) * ( metricVal[i][j] - metricVal[NUM_INPUTS+2][j] );
      }
      if (i >= 20 && i < NUM_INPUTS) {
	metricStdev[j][3] = metricStdev[j][3] + ( metricVal[i][j] - metricVal[NUM_INPUTS+3][j] ) * ( metricVal[i][j] - metricVal[NUM_INPUTS+3][j] );
      }
    }
  }
  for (int j=0; j<NUM_METRICS; j++) {
    metricStdev[j][0] = sqrt(metricStdev[j][0] / (NUM_INPUTS - 1));
    metricStdev[j][1] = sqrt(metricStdev[j][1] / (10 - 1));
    metricStdev[j][2] = sqrt(metricStdev[j][2] / (10 - 1));
    metricStdev[j][3] = sqrt(metricStdev[j][3] / (28 - 1));
  }

  float metricRepVal[NUM_REPS];
  float average[2] = {0.0,0.0};
  float stdev[2] = {0.0,0.0};
  float cov[2];
  for (int i=0; i<(NUM_REPS); i++)  { metricRepVal[i] = 0.0;} 
  for (int i=0; i<NUM_REPS; i++) {
    fgets(line,1000,metricFile);
    sscanf(line, " %s  %s %s %f; %f; %f; %f; %f; %f; %f; %f; %f ", 
	   username, segFilename, refFilename,
      &volumeSeg, &volumeRef, &volumeDif, &volumeDifPerc, &volumeDifPercAbs,
	    &avgDistance, &rmsDistance, &maxDistance, &tanimotoError );
    metricRepVal[i] = volumeSeg * 1000;    average[i % 2] += volumeSeg * 1000;
  }

  fclose( metricFile );

  average[0] /= NUM_REPS/2;
  average[1] /= NUM_REPS/2;

  //calculate standard devation for reps
  for (int i=0; i<NUM_REPS; i++) {
    stdev[i % 2] = stdev[i % 2] + ( metricRepVal[i] - average[i % 2] ) * ( metricRepVal[i] - average[i % 2] );
  }
  stdev[0] = sqrt(stdev[0] / ( NUM_REPS/2 - 1 )); 
  stdev[1] = sqrt(stdev[1] / ( NUM_REPS/2 - 1 )); 
  cov[0] = stdev[0] / average [0] * 100;
  cov[1] = stdev[1] / average [1] * 100;
  float TotalCov = (cov[0] + cov[1] ) / 2;

  // --------------------------------------------------------------------------------------------
  // Load all scores
  // --------------------------------------------------------------------------------------------

  // store all scores and average here:
  float scoreVal[NUM_INPUTS+4][NUM_METRICS+1];
  for (int i=0; i<(NUM_INPUTS+4); i++) for (int j=0; j<(NUM_METRICS+1); j++) scoreVal[i][j] = 0.0;
  float scoreStdev[4];
  for (int i=0; i < 4; i++) scoreStdev[i] = 0;
  
  FILE *scoreFile = fopen( argv[2], "r" );
  if (!scoreFile) {
    printf( "ERROR: Could not open %s\n", argv[1] );
    return -1;
  }
  
  float scoreVOE, scoreRVD, scoreASD, scoreRMSSD, scoreMSSD, totalScore;
	
  for (int i=0; i<NUM_INPUTS; i++) {
    fgets(line,1000, scoreFile);
    sscanf(line, " %s %f; %f; %f; %f; %f; %f ", 
	   segFilename, &scoreVOE, &scoreRVD, &scoreASD, &scoreRMSSD, &scoreMSSD, &totalScore );
    scoreVal[i][0] = scoreVOE;     scoreVal[NUM_INPUTS][0] += scoreVOE;
    scoreVal[i][1] = scoreRVD;     scoreVal[NUM_INPUTS][1] += scoreRVD;
    scoreVal[i][2] = scoreASD;     scoreVal[NUM_INPUTS][2] += scoreASD;
    scoreVal[i][3] = scoreRMSSD;   scoreVal[NUM_INPUTS][3] += scoreRMSSD;
    scoreVal[i][4] = scoreMSSD;    scoreVal[NUM_INPUTS][4] += scoreMSSD;
    scoreVal[i][5] = totalScore;   scoreVal[NUM_INPUTS][5] += totalScore;
    if (i < 10) {
      scoreVal[NUM_INPUTS+1][0] += scoreVOE;
      scoreVal[NUM_INPUTS+1][1] += scoreRVD;
      scoreVal[NUM_INPUTS+1][2] += scoreASD;
      scoreVal[NUM_INPUTS+1][3] += scoreRMSSD;
      scoreVal[NUM_INPUTS+1][4] += scoreMSSD;
      scoreVal[NUM_INPUTS+1][5] += totalScore;
    }
    if (i >= 10 && i < 20) {
      scoreVal[NUM_INPUTS+2][0] += scoreVOE;
      scoreVal[NUM_INPUTS+2][1] += scoreRVD;
      scoreVal[NUM_INPUTS+2][2] += scoreASD;
      scoreVal[NUM_INPUTS+2][3] += scoreRMSSD;
      scoreVal[NUM_INPUTS+2][4] += scoreMSSD;
      scoreVal[NUM_INPUTS+2][5] += totalScore;
    }
    if (i >= 20 && i < NUM_INPUTS) {
      scoreVal[NUM_INPUTS+3][0] += scoreVOE;
      scoreVal[NUM_INPUTS+3][1] += scoreRVD;
      scoreVal[NUM_INPUTS+3][2] += scoreASD;
      scoreVal[NUM_INPUTS+3][3] += scoreRMSSD;
      scoreVal[NUM_INPUTS+3][4] += scoreMSSD;
      scoreVal[NUM_INPUTS+3][5] += totalScore;
    }
  }
  fclose( scoreFile );
  // calculate average
  for (int j=0; j<(NUM_METRICS+1); j++) scoreVal[NUM_INPUTS][j] /= NUM_INPUTS;
  // calulate UNC PED average
  for (int j=0; j<(NUM_METRICS+1); j++) scoreVal[NUM_INPUTS+1][j] /= 10;
  // calulate UNC PARK average
  for (int j=0; j<(NUM_METRICS+1); j++) scoreVal[NUM_INPUTS+2][j] /= 10;
  // calulate BWH average
  for (int j=0; j<(NUM_METRICS+1); j++) scoreVal[NUM_INPUTS+3][j] /= 28;

  for (int i=0; i<NUM_INPUTS; i++) {
    scoreStdev[0] = scoreStdev[0] + ( scoreVal[i][5] - scoreVal[NUM_INPUTS+0][5] ) * ( scoreVal[i][5] - scoreVal[NUM_INPUTS+0][5] );
    
    if (i < 10) {
      scoreStdev[1] = scoreStdev[1] + ( scoreVal[i][5] - scoreVal[NUM_INPUTS+1][5] ) * ( scoreVal[i][5] - scoreVal[NUM_INPUTS+1][5] );
    }
    if (i >= 10 && i < 20) {
      scoreStdev[2] = scoreStdev[2] + ( scoreVal[i][5] - scoreVal[NUM_INPUTS+2][5] ) * ( scoreVal[i][5] - scoreVal[NUM_INPUTS+2][5] );
    }
    if (i >= 20 && i < NUM_INPUTS) {
      scoreStdev[3] = scoreStdev[3] + ( scoreVal[i][5] - scoreVal[NUM_INPUTS+3][5] ) * ( scoreVal[i][5] - scoreVal[NUM_INPUTS+3][5] );
    }
  }
  scoreStdev[0] = sqrt(scoreStdev[0] / (NUM_INPUTS - 1));
  scoreStdev[1] = sqrt(scoreStdev[1] / (10 - 1));
  scoreStdev[2] = sqrt(scoreStdev[2] / (10 - 1));
  scoreStdev[3] = sqrt(scoreStdev[3] / (28 - 1));
               
  // echo " CorrTab = quit [[ val,val2 ],[val3,val4]];" >!  $matlabTmp
  std::string matlabFile = std::string("matlab_script.m");
  FILE * script = fopen (matlabFile.c_str(),"w");
  // UNC Ped
  fprintf(script, "CorrTabLeft = [[");
  for (int i = 0; i < 10; i += 2) {
    if (i > 1)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 0; i < 10; i += 2) {
    if (i > 1)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "pLeft = corr(CorrTabLeft, 'type', 'Pearson');\ndisplay(['CorrelLeft: ' num2str(pLeft(2))]);\n");
  fprintf(script, "CorrTabRight = [[");
  for (int i = 1; i < 10; i += 2) {
    if (i > 1)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 1; i < 10; i += 2) {
    if (i > 1)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "pRight = corr(CorrTabRight, 'type', 'Pearson');\ndisplay(['CorrelRight: ' num2str(pRight(2))]);\n");
  // UNC Eld
  fprintf(script, "CorrTabLeft = [[");
  for (int i = 10; i < 20; i += 2) {
    if (i > 11)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 10; i < 20; i += 2) {
    if (i > 11)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "pLeft = corr(CorrTabLeft, 'type', 'Pearson');\ndisplay(['CorrelLeft: ' num2str(pLeft(2))]);\n");
  fprintf(script, "CorrTabRight = [[");
  for (int i = 11; i < 20; i += 2) {
    if (i > 11)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 11; i < 20; i += 2) {
    if (i > 11)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "pRight = corr(CorrTabRight, 'type', 'Pearson');\ndisplay(['CorrelRight: ' num2str(pRight(2))]);\n");
  // BWH
  fprintf(script, "CorrTabLeft = [[");
  for (int i = 20; i < 48; i += 2) {
    if (i > 21)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 20; i < 48; i += 2) {
    if (i > 21)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "pLeft = corr(CorrTabLeft, 'type', 'Pearson');\ndisplay(['CorrelLeft: ' num2str(pLeft(2))]);\n");
  fprintf(script, "CorrTabRight = [[");
  for (int i = 21; i < 48; i += 2) {
    if (i > 21)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 21; i < 48; i += 2) {
    if (i > 21)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "pRight = corr(CorrTabRight, 'type', 'Pearson');\ndisplay(['CorrelRight: ' num2str(pRight(2))]);\n");
  // for the 3 groups combining left and right
  // -> more data, but difference between left and right may not be what we are interested in
  fprintf(script, "CorrTab = [[");
  for (int i = 1; i < 10; i ++) {
    if (i > 0)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 1; i < 10; i ++) {
    if (i > 0)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "p = corr(CorrTab, 'type', 'Pearson');\ndisplay(['Correl: ' num2str(p(2))]);\n");
  fprintf(script, "CorrTab = [[");
  for (int i = 11; i < 20; i ++) {
    if (i > 10)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 11; i < 20; i ++) {
    if (i > 10)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "p = corr(CorrTab, 'type', 'Pearson');\ndisplay(['Correl: ' num2str(p(2))]);\n");
  fprintf(script, "CorrTab = [[");
  for (int i = 21; i < 48; i ++) {
    if (i > 20)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeSeg[i]);
  }
  fprintf(script, "],[");
  for (int i = 21; i < 48; i ++) {
    if (i > 20)    fprintf(script, "; ");
    fprintf(script, "%lf",allVolumeRef[i]);
  }
  fprintf(script, "]];\n");
  fprintf(script, "p = corr(CorrTab, 'type', 'Pearson');\ndisplay(['Correl: ' num2str(p(2))]);\n");
  fprintf(script, "quit;\n\n");
  fclose(script);
  std::string command = std::string("matlab -r matlab_script -nodesktop -nosplash -nojvm -nodisplay | grep Correl > /tmp/matlabOutput.txt");
  system(command.c_str());

  float leftCorrel[3], rightCorrel[3];
  FILE * correlfile = fopen ("/tmp/matlabOutput.txt","r");
  fscanf(correlfile, " CorrelLeft: %f CorrelRight: %f ", &(leftCorrel[0]), &(rightCorrel[0]));
  fscanf(correlfile, " CorrelLeft: %f CorrelRight: %f ", &(leftCorrel[1]), &(rightCorrel[1]));
  fscanf(correlfile, " CorrelLeft: %f CorrelRight: %f ", &(leftCorrel[2]), &(rightCorrel[2]));
  fclose(correlfile);

  // --------------------------------------------------------------------------------------------
  // Write output
  // --------------------------------------------------------------------------------------------

  char buffer[1024];

  // save output with this name
  char resultsName[1024];
  strcpy( resultsName, "results.tex" );
  for (int i=3; i<argc; i++) {
    if ( strcmp( argv[i], "-o" )==0 && i<(argc-1)) {
      strncpy( resultsName, argv[i+1], 1024 );  
      break;
    }
  }
  { 
    FILE *results = fopen( resultsName, "w" );

    // write results table
    fputs( "\
\\documentclass{llncs}\n			\
\\usepackage{amssymb}\n				\
\\usepackage{amsmath}\n\
\\pagestyle{empty}\n\
\\usepackage{geometry}\n\
\\usepackage{graphicx}\n\n\
\\newsavebox{\\NEWbox}\n\
\\newcommand{\\NEWscalefactor}{0.93}\n\
\\newlength{\\NEWwidth}\n\
\\newlength{\\NEWheight}\n\
\\newlength{\\NEWdepth}\n\n\
\\setlength{\\topskip}{0pt}\n\
\\setlength{\\parindent}{0pt}\n\
\\setlength{\\abovedisplayskip}{0pt}\n\
\\setlength{\\belowdisplayskip}{0pt}\n\
\\normalfont\n\n\
\\begin{lrbox}{\\NEWbox}\n\
\\begin{tabular}{|l|rr|rr|rr|rr|rr|r|}\n\
\\hline\n\
All Dataset &  \\multicolumn{2}{c|}{Overlap Err} & \\multicolumn{2}{c|}{Volume Diff.} & \\multicolumn{2}{c|}{Abs. Dist.} &\n\
          \\multicolumn{2}{c|}{RMS Dist.} & \\multicolumn{2}{c|}{Max. Dist.} & Total \\\\\n\
        & ~~~[\\%] & Score & ~~~[\\%] & Score & ~[mm] & Score & ~[mm] & Score & ~[mm] & Score \n\
        & ~Score \\\\ \n \\hline \n" , results );
    for (int i=0; i<NUM_INPUTS; i+=2) {
      sprintf( buffer, "%s  & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ &  %4.1f &  %3.f~ &  %4.1f &  %3.f~ & %3.f~ \\\\\n",
	       datasetName[i/2], 
	       (metricVal[i][0] + metricVal[i+1][0])/2, (scoreVal[i][0] + scoreVal[i+1][0])/2, 
	       (metricVal[i][1] + metricVal[i+1][1])/2, (scoreVal[i][1] + scoreVal[i+1][1])/2, 
	       (metricVal[i][2] + metricVal[i+1][2])/2, (scoreVal[i][2] + scoreVal[i+1][2])/2,
	       (metricVal[i][3] + metricVal[i+1][3])/2, (scoreVal[i][3] + scoreVal[i+1][3])/2,
	       (metricVal[i][4] + metricVal[i+1][4])/2, (scoreVal[i][4] + scoreVal[i+1][4])/2,
	       (scoreVal[i][5] + scoreVal[i+1][5])/2);
      fputs( buffer, results );
    }
    fputs( "\\hline\\hline\n", results );
    sprintf( buffer, "Average All & %5.f $\\pm$ %5.f &  %3.f~ & %5.f $\\pm$ %5.f &  %3.f~ & %5.f $\\pm$ %5.f &  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ & %3.f~ $\\pm$ %3.f~ \\\\\n \\hline ", 
	     metricVal[NUM_INPUTS][0], metricStdev[0][0], scoreVal[NUM_INPUTS][0], metricVal[NUM_INPUTS][1], metricStdev[1][0], scoreVal[NUM_INPUTS][1], 
	     metricVal[NUM_INPUTS][2], metricStdev[2][0], scoreVal[NUM_INPUTS][2], metricVal[NUM_INPUTS][3], metricStdev[3][0], scoreVal[NUM_INPUTS][3], 
	     metricVal[NUM_INPUTS][4], metricStdev[4][0], scoreVal[NUM_INPUTS][4], scoreVal[NUM_INPUTS][5], scoreStdev[0]);
    fputs( buffer, results );
    sprintf( buffer, "Average UNC Ped & %5.f  $\\pm$ %5.f &  %3.f~ & %5.f  $\\pm$ %5.f &  %3.f~ & %5.f  $\\pm$ %5.f &  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ & %3.f~ $\\pm$ %3.f~ \\\\\n", 
	     metricVal[NUM_INPUTS+1][0], metricStdev[0][1], scoreVal[NUM_INPUTS+1][0], metricVal[NUM_INPUTS+1][1], metricStdev[1][1],scoreVal[NUM_INPUTS+1][1], 
	     metricVal[NUM_INPUTS+1][2], metricStdev[2][1],scoreVal[NUM_INPUTS+1][2], metricVal[NUM_INPUTS+1][3], metricStdev[3][1],scoreVal[NUM_INPUTS+1][3], 
	     metricVal[NUM_INPUTS+1][4], metricStdev[4][1], scoreVal[NUM_INPUTS+1][4], scoreVal[NUM_INPUTS+1][5] , scoreStdev[1]);
    fputs( buffer, results );
    sprintf( buffer, "Average UNC Eld&  %5.f  $\\pm$ %5.f&  %3.f~ & %5.f  $\\pm$ %5.f&  %3.f~ & %5.f  $\\pm$ %5.f&  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ & %3.f~ $\\pm$ %3.f~ \\\\\n", 
	     metricVal[NUM_INPUTS+2][0], metricStdev[0][2], scoreVal[NUM_INPUTS+2][0], metricVal[NUM_INPUTS+2][1], metricStdev[1][2],scoreVal[NUM_INPUTS+2][1], 
	     metricVal[NUM_INPUTS+2][2], metricStdev[2][2],scoreVal[NUM_INPUTS+2][2], metricVal[NUM_INPUTS+2][3], metricStdev[2][2],scoreVal[NUM_INPUTS+2][3], 
	     metricVal[NUM_INPUTS+2][4], metricStdev[4][2], scoreVal[NUM_INPUTS+2][4], scoreVal[NUM_INPUTS+2][5] , scoreStdev[2]);
    fputs( buffer, results );
    sprintf( buffer, "Average BWH PNL& %5.f $\\pm$ %5.f&  %3.f~ & %5.f  $\\pm$ %5.f&  %3.f~ & %5.f  $\\pm$ %5.f&  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ &  %4.f $\\pm$ %4.f &  %3.f~ & %3.f~ $\\pm$ %3.f~ \\\\\n", 
	     metricVal[NUM_INPUTS+3][0], metricStdev[0][3], scoreVal[NUM_INPUTS+3][0], metricVal[NUM_INPUTS+3][1], metricStdev[1][3],scoreVal[NUM_INPUTS+3][1], 
	     metricVal[NUM_INPUTS+3][2], metricStdev[2][3], scoreVal[NUM_INPUTS+3][2], metricVal[NUM_INPUTS+3][3], metricStdev[3][3],scoreVal[NUM_INPUTS+3][3], 
	     metricVal[NUM_INPUTS+3][4], metricStdev[4][3], scoreVal[NUM_INPUTS+3][4], scoreVal[NUM_INPUTS+3][5] , scoreStdev[3]);
    fputs( buffer, results );
    fputs( "\
\\hline\n					\
\\end{tabular}\n				\
\\end{lrbox}\n\n							\
\\settowidth{\\NEWwidth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n \
\\settoheight{\\NEWheight}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n \
\\settodepth{\\NEWdepth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n \
\\newwrite\\foo \\immediate\\openout\\foo=\\jobname.sizes \\immediate\\write\\foo{\\the\\NEWdepth (Depth)}\n\
\\immediate\\write\\foo{\\the\\NEWheight (Height)}\n\
\\addtolength{\\NEWheight}{\\NEWdepth}\n\
\\addtolength{\\NEWheight}{2.880000 pt}\n\
\\addtolength{\\NEWheight}{10.000000 pt}\n\
\\addtolength{\\NEWwidth}{10.000000 pt}\n\
\\immediate\\write\\foo{\\the\\NEWheight (TotalHeight)} \\immediate\\write\\foo{\\the\\NEWwidth (Width)}\n\
\\closeout\\foo \\geometry{paperwidth=\\NEWwidth,paperheight=\\NEWheight,margin=0pt,left=5.000000 pt,top=6.440000 pt}\n\
\\begin{document}\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}\\end{document}\n\
  ", results );
    
    fclose( results );
    
  }
  {
    std::string results2Name;
    char * texPtr = strstr(resultsName, ".tex");
    if (texPtr) texPtr[0] = '\0';
    results2Name = std::string(resultsName) + "AllData.tex";
    FILE *results = fopen( results2Name.c_str(), "w" );
    // write results table
    fputs( "\
\\documentclass{llncs}\n			\
\\usepackage{amssymb}\n\
\\usepackage{amsmath}\n\
\\pagestyle{empty}\n\
\\usepackage{geometry}\n\
\\usepackage{graphicx}\n\n\
\\newsavebox{\\NEWbox}\n\
\\newcommand{\\NEWscalefactor}{0.93}\n\
\\newlength{\\NEWwidth}\n\
\\newlength{\\NEWheight}\n\
\\newlength{\\NEWdepth}\n\n\
\\setlength{\\topskip}{0pt}\n\
\\setlength{\\parindent}{0pt}\n\
\\setlength{\\abovedisplayskip}{0pt}\n\
\\setlength{\\belowdisplayskip}{0pt}\n\
\\normalfont\n\n\
\\begin{lrbox}{\\NEWbox}\n\
\\begin{tabular}{|l|c|rr|rr|rr|rr|rr|r|}\n\
\\hline\n\
All Dataset &  Side & \\multicolumn{2}{c|}{Overlap Err} & \\multicolumn{2}{c|}{Volume Diff.} & \\multicolumn{2}{c|}{Abs. Dist.} &\n\
          \\multicolumn{2}{c|}{RMS Dist.} & \\multicolumn{2}{c|}{Max. Dist.} & Total \\\\\n\
        & & ~~~[\\%] & Score & ~~~[\\%] & Score & ~[mm] & Score & ~[mm] & Score & ~[mm] & Score \n\
        & ~Score \\\\ \n \\hline \n" , results );
    for (int i=0; i<NUM_INPUTS; i+=2) {
      sprintf( buffer, "%s & Left & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ &  %4.1f &  %3.f~ &  %4.1f &  %3.f~ & %3.f~ \\\\\n",
	       datasetName[i/2], metricVal[i][0], scoreVal[i][0], metricVal[i][1], scoreVal[i][1], metricVal[i][2], scoreVal[i][2], metricVal[i][3], scoreVal[i][3], metricVal[i][4], scoreVal[i][4], scoreVal[i][5]);
      fputs( buffer, results );
    }
    fputs( "\\hline\n", results );
    for (int i=1; i<NUM_INPUTS; i+=2) {
      sprintf( buffer, "%s & Right & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ &  %4.1f &  %3.f~ &  %4.1f &  %3.f~ & %3.f~ \\\\\n",
	       datasetName[(i-1)/2], metricVal[i][0], scoreVal[i][0], metricVal[i][1], scoreVal[i][1], metricVal[i][2], scoreVal[i][2], metricVal[i][3], scoreVal[i][3], metricVal[i][4], scoreVal[i][4], scoreVal[i][5]);
      fputs( buffer, results );
    }
    fputs( "\\hline\\hline\n", results );
    sprintf( buffer, "Average All & Average & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ &  %4.1f &  %3.f~ &  %4.1f &  %3.f~ & %3.f~ \\\\\n \\hline ", 
	     metricVal[NUM_INPUTS][0], scoreVal[NUM_INPUTS][0], metricVal[NUM_INPUTS][1], scoreVal[NUM_INPUTS][1], 
	     metricVal[NUM_INPUTS][2], scoreVal[NUM_INPUTS][2], metricVal[NUM_INPUTS][3], scoreVal[NUM_INPUTS][3], 
	     metricVal[NUM_INPUTS][4], scoreVal[NUM_INPUTS][4], scoreVal[NUM_INPUTS][5] );
    fputs( buffer, results );
    sprintf( buffer, "Average UNC Ped& Average & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ &  %4.1f &  %3.f~ &  %4.1f &  %3.f~ & %3.f~ \\\\\n", 
	     metricVal[NUM_INPUTS+1][0], scoreVal[NUM_INPUTS+1][0], metricVal[NUM_INPUTS+1][1], scoreVal[NUM_INPUTS+1][1], 
	     metricVal[NUM_INPUTS+1][2], scoreVal[NUM_INPUTS+1][2], metricVal[NUM_INPUTS+1][3], scoreVal[NUM_INPUTS+1][3], 
	     metricVal[NUM_INPUTS+1][4], scoreVal[NUM_INPUTS+1][4], scoreVal[NUM_INPUTS+1][5] );
    fputs( buffer, results );
    sprintf( buffer, "Average UNC Eld& Average & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ &  %4.1f &  %3.f~ &  %4.1f &  %3.f~ & %3.f~ \\\\\n", 
	     metricVal[NUM_INPUTS+2][0], scoreVal[NUM_INPUTS+2][0], metricVal[NUM_INPUTS+2][1], scoreVal[NUM_INPUTS+2][1], 
	     metricVal[NUM_INPUTS+2][2], scoreVal[NUM_INPUTS+2][2], metricVal[NUM_INPUTS+2][3], scoreVal[NUM_INPUTS+2][3], 
	     metricVal[NUM_INPUTS+2][4], scoreVal[NUM_INPUTS+2][4], scoreVal[NUM_INPUTS+2][5] );
    fputs( buffer, results );
    sprintf( buffer, "Average BWH PNL& Average & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ & %5.1f  &  %3.f~ &  %4.1f &  %3.f~ &  %4.1f &  %3.f~ & %3.f~ \\\\\n", 
	     metricVal[NUM_INPUTS+3][0], scoreVal[NUM_INPUTS+3][0], metricVal[NUM_INPUTS+3][1], scoreVal[NUM_INPUTS+3][1], 
	     metricVal[NUM_INPUTS+3][2], scoreVal[NUM_INPUTS+3][2], metricVal[NUM_INPUTS+3][3], scoreVal[NUM_INPUTS+3][3], 
	     metricVal[NUM_INPUTS+3][4], scoreVal[NUM_INPUTS+3][4], scoreVal[NUM_INPUTS+3][5] );
    fputs( buffer, results );
    fputs( "\
\\hline\n					\
\\end{tabular}\n				\
\\end{lrbox}\n\n							\
\\settowidth{\\NEWwidth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n\
\\settoheight{\\NEWheight}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n\
\\settodepth{\\NEWdepth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n\
\\newwrite\\foo \\immediate\\openout\\foo=\\jobname.sizes \\immediate\\write\\foo{\\the\\NEWdepth (Depth)}\n\
\\immediate\\write\\foo{\\the\\NEWheight (Height)}\n\
\\addtolength{\\NEWheight}{\\NEWdepth}\n\
\\addtolength{\\NEWheight}{2.880000 pt}\n\
\\addtolength{\\NEWheight}{10.000000 pt}\n\
\\addtolength{\\NEWwidth}{10.000000 pt}\n\
\\immediate\\write\\foo{\\the\\NEWheight (TotalHeight)} \\immediate\\write\\foo{\\the\\NEWwidth (Width)}\n\
\\closeout\\foo \\geometry{paperwidth=\\NEWwidth,paperheight=\\NEWheight,margin=0pt,left=5.000000 pt,top=6.440000 pt}\n\
\\begin{document}\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}\\end{document}\n\
  ", results );
    
    fclose( results );
  }
  {
    std::string results2Name;
    char * texPtr = strstr(resultsName, ".tex");
    if (texPtr) texPtr[0] = '\0';
    results2Name = std::string(resultsName) + "Reliability.tex";
    FILE *results = fopen( results2Name.c_str(), "w" );
    
    // write results table
    fputs( "\
\\documentclass{llncs}\n			\
\\usepackage{amssymb}\n				\
\\usepackage{amsmath}\n				\
\\pagestyle{empty}\n\
\\usepackage{geometry}\n\
\\usepackage{graphicx}\n\n\
\\newsavebox{\\NEWbox}\n\
\\newcommand{\\NEWscalefactor}{0.93}\n\
\\newlength{\\NEWwidth}\n\
\\newlength{\\NEWheight}\n\
\\newlength{\\NEWdepth}\n\n\
\\setlength{\\topskip}{0pt}\n\
\\setlength{\\parindent}{0pt}\n\
\\setlength{\\abovedisplayskip}{0pt}\n\
\\setlength{\\belowdisplayskip}{0pt}\n\
\\normalfont\n\n\
\\begin{lrbox}{\\NEWbox}\n\
\\begin{tabular}{|l||c|c|c|c|c|c|c|c|c|c||c||c||c||}\n\
\\hline\n\
Test/Re-Test & UNC 03 & UNC 04&UNC 09&UNC 11&UNC 17&UNC 18&UNC 21&UNC 22&UNC 24&UNC 25  & Mean & Stdev & COV \\\\\n" , results);
    fputs( "&  [mm$^3$] &  [mm$^3$] & [mm$^3$] & [mm$^3$] & [mm$^3$]& [mm$^3$]& [mm$^3$]& [mm$^3$]& [mm$^3$]& [mm$^3$] & [mm$^3$] &[mm$^3$] & [\\%] \\\\ \n \\hline \n" , results );
    fputs( "Left  ", results );
    for (int i=0; i<NUM_REPS; i+=2) {
      sprintf( buffer," & %5.0f  ",  metricRepVal[i] );
      fputs( buffer, results );
    }
    sprintf( buffer," & %5.0f & %5.0f& %5.1f ",  average[0], stdev[0],cov[0] );
    fputs( buffer, results );
    fputs( "\\\\\n Right  ", results );
    for (int i=1; i<NUM_REPS; i+=2) {
      sprintf( buffer," & %5.0f  ",  metricRepVal[i] );
      fputs( buffer, results );
    }
    sprintf( buffer," & %5.0f & %5.0f& %5.1f ",  average[1], stdev[1],cov[1] );
    fputs( buffer, results );
    fputs( "\\\\\n \\hline Total & \\multicolumn{10}{c||}{-} & - & - &", results );
    
    sprintf( buffer," %5.1f ",  TotalCov );
    fputs( buffer, results );
    fputs( "\\\\\n \\hline\n\
\\end{tabular}\n	    \
\\end{lrbox}\n\n							\
\\settowidth{\\NEWwidth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n \
\\settoheight{\\NEWheight}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n\
\\settodepth{\\NEWdepth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n\
\\newwrite\\foo \\immediate\\openout\\foo=\\jobname.sizes \\immediate\\write\\foo{\\the\\NEWdepth (Depth)}\n\
\\immediate\\write\\foo{\\the\\NEWheight (Height)}\n\
\\addtolength{\\NEWheight}{\\NEWdepth}\n\
\\addtolength{\\NEWheight}{2.880000 pt}\n\
\\addtolength{\\NEWheight}{10.000000 pt}\n\
\\addtolength{\\NEWwidth}{10.000000 pt}\n\
\\immediate\\write\\foo{\\the\\NEWheight (TotalHeight)} \\immediate\\write\\foo{\\the\\NEWwidth (Width)}\n\
\\closeout\\foo \\geometry{paperwidth=\\NEWwidth,paperheight=\\NEWheight,margin=0pt,left=5.000000 pt,top=6.440000 pt}\n\
\\begin{document}\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}\\end{document}\n\
  ", results );
  
    fclose( results );
  }  
  {
    std::string results2Name;
    char * texPtr = strstr(resultsName, ".tex");
    if (texPtr) texPtr[0] = '\0';
    results2Name = std::string(resultsName) + "Correlation.tex";
    FILE *results = fopen( results2Name.c_str(), "w" );
    
    // write results table
    fputs( "\
\\documentclass{llncs}\n			\
\\usepackage{amssymb}\n				\
\\usepackage{amsmath}\n				\
\\pagestyle{empty}\n\
\\usepackage{geometry}\n\
\\usepackage{graphicx}\n\n\
\\newsavebox{\\NEWbox}\n\
\\newcommand{\\NEWscalefactor}{0.93}\n\
\\newlength{\\NEWwidth}\n\
\\newlength{\\NEWheight}\n\
\\newlength{\\NEWdepth}\n\n\
\\setlength{\\topskip}{0pt}\n\
\\setlength{\\parindent}{0pt}\n\
\\setlength{\\abovedisplayskip}{0pt}\n\
\\setlength{\\belowdisplayskip}{0pt}\n\
\\normalfont\n\n\
\\begin{lrbox}{\\NEWbox}\n\
\\begin{tabular}{|l|r|r|r||r||}\n\
\\hline\n\
Correl & UNC Ped & UNC Eld & BWH PNL& Total \\\\\n \\hline " , results);
    sprintf( buffer," Left & %5.4f & %5.4f & %5.4f & %5.4f \\\\\n",  
	     leftCorrel[0], leftCorrel[1],  leftCorrel[2],
	     (leftCorrel[0] + leftCorrel[1] + leftCorrel[2])/3);
    fputs( buffer, results );
    sprintf( buffer," Right & %5.4f & %5.4f & %5.4f & %5.4f \\\\\n",  
	     rightCorrel[0], rightCorrel[1],  rightCorrel[2],
	     (rightCorrel[0] + rightCorrel[1] + rightCorrel[2])/3); 
    fputs( buffer, results );
    sprintf( buffer," \\hline Average & %5.4f & %5.4f & %5.4f & %5.4f \\\\\n",  
	     (leftCorrel[0] + rightCorrel[0])/2, 
	     (leftCorrel[1] + rightCorrel[1])/2,  
	     (leftCorrel[2] + rightCorrel[2])/2,
	     (rightCorrel[0] + rightCorrel[1] + rightCorrel[2]+
	      leftCorrel[0] + leftCorrel[1] + leftCorrel[2])/6);
    fputs( buffer, results );
    fputs( "\\hline\n\
\\end{tabular}\n	    \
\\end{lrbox}\n\n							\
\\settowidth{\\NEWwidth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n \
\\settoheight{\\NEWheight}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n\
\\settodepth{\\NEWdepth}{\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}}\n\
\\newwrite\\foo \\immediate\\openout\\foo=\\jobname.sizes \\immediate\\write\\foo{\\the\\NEWdepth (Depth)}\n\
\\immediate\\write\\foo{\\the\\NEWheight (Height)}\n\
\\addtolength{\\NEWheight}{\\NEWdepth}\n\
\\addtolength{\\NEWheight}{2.880000 pt}\n\
\\addtolength{\\NEWheight}{10.000000 pt}\n\
\\addtolength{\\NEWwidth}{10.000000 pt}\n\
\\immediate\\write\\foo{\\the\\NEWheight (TotalHeight)} \\immediate\\write\\foo{\\the\\NEWwidth (Width)}\n\
\\closeout\\foo \\geometry{paperwidth=\\NEWwidth,paperheight=\\NEWheight,margin=0pt,left=5.000000 pt,top=6.440000 pt}\n\
\\begin{document}\\scalebox{\\NEWscalefactor}{\\usebox{\\NEWbox}}\\end{document}\n\
  ", results );
  
    fclose( results );
  }  

  { 
    std::string results2Name;
    char * texPtr = strstr(resultsName, ".tex");
    if (texPtr) texPtr[0] = '\0';
    results2Name = std::string(resultsName) + "Plot.txt";
    FILE *results = fopen( results2Name.c_str(), "w" );
    
    
    for (int i=0; i<NUM_INPUTS/2; i++) {
      fprintf(results, "%d %5.1lf\n", i, (double) (scoreVal[i*2 + 0][5] + scoreVal[i*2 + 1][5])/2);
    }
    fclose(results);
  }
  return 0;
}
