#include "FeatureSelection.h"

#include "../include/FEAST/FSAlgorithms.h"
#include "../include/FEAST/FSToolbox.h"

/* MIToolbox includes */
#include "../include/FEAST/MutualInformation.h"
#include "../include/FEAST/ArrayOperations.h"
#include "../include/FEAST/Entropy.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "../include/MatrixOp.h"

using std::cout;
using std::endl;
using std::vector;
using std::swap;

double FeatureSelection::eu_distance(vector<double>& x, vector<double>& y)
{
    if(x.size() != y.size())
        return -1;
    double sum = 0.0;
    for(unsigned i=0; i<x.size(); i++){
        sum += pow(x[i]-y[i], 2);
    }
    return sqrt(sum);
}

void FeatureSelection::JMI(int k, int noOfSamples, int noOfFeatures,double *featureMatrix,
                           double *classColumn, vector<int> &outputId)
{
    double *outputFeatures = (double*)calloc(k, sizeof(double));

    /*holds the class MI values*/
    double *classMI = (double *)calloc(noOfFeatures,sizeof(double));

    char *selectedFeatures = (char *)calloc(noOfFeatures,sizeof(char));

    /*holds the intra feature MI values*/
    int sizeOfMatrix = k*noOfFeatures;
    double *featureMIMatrix = (double *)calloc(sizeOfMatrix,sizeof(double));

    double maxMI = 0.0;
    int maxMICounter = -1;

    double **feature2D = (double**) calloc(noOfFeatures,sizeof(double*));

    double score, currentScore; //totalFeatureMI;
    int currentHighestFeature;

    double *mergedVector = (double *) calloc(noOfSamples,sizeof(double));

    int arrayPosition;
    double mi;

    int i,j,x;

    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    for (i = 0; i < sizeOfMatrix; i++) {
        featureMIMatrix[i] = -1;
    }/*for featureMIMatrix - blank to -1*/


    for (i = 0; i < noOfFeatures; i++) {
        /*calculate mutual info
        **double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);
        */
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);
        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    /*****************************************************************************
    ** We have populated the classMI array, and selected the highest
    ** MI feature as the first output feature
    ** Now we move into the JMI algorithm
    *****************************************************************************/

    for (i = 1; i < k; i++) {
        score = 0.0;
        currentHighestFeature = 0;
        currentScore = 0.0;
        //totalFeatureMI = 0.0;

        for (j = 0; j < noOfFeatures; j++) {

            /*if we haven't selected j*/
            if (selectedFeatures[j] == 0) {
                currentScore = 0.0;
                //totalFeatureMI = 0.0;

                for (x = 0; x < i; x++) {
                    arrayPosition = x*noOfFeatures + j;
                    if (featureMIMatrix[arrayPosition] == -1) {
                        mergeArrays(feature2D[(int) outputFeatures[x]], feature2D[j],mergedVector,noOfSamples);
                        /*double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);*/
                        mi = calculateMutualInformation(mergedVector, classColumn, noOfSamples);

                        featureMIMatrix[arrayPosition] = mi;
                    }/*if not already known*/
                    currentScore += featureMIMatrix[arrayPosition];
                }/*for the number of already selected features*/

                if (currentScore > score) {
                    score = currentScore;
                    currentHighestFeature = j;
                }
            }/*if j is unselected*/
        }/*for number of features*/

        selectedFeatures[currentHighestFeature] = 1;
        outputFeatures[i] = currentHighestFeature;

    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(feature2D);
    FREE_FUNC(featureMIMatrix);
    FREE_FUNC(mergedVector);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    feature2D = NULL;
    featureMIMatrix = NULL;
    mergedVector = NULL;
    selectedFeatures = NULL;

    for(int i=0; i<k; i++) {
        if(outputFeatures[i]>=0) outputId.push_back(outputFeatures[i]);
    }
    free(outputFeatures);
}

void FeatureSelection::MRMR(int k, int noOfSamples, int noOfFeatures, double *featureMatrix,
                            double *classColumn, vector<int> &outputId)
{
    double *outputFeatures = (double*)calloc(k, sizeof(double));
    double **feature2D = (double**) checkedCalloc(noOfFeatures,sizeof(double*));
    /*holds the class MI values*/
    double *classMI = (double *)checkedCalloc(noOfFeatures,sizeof(double));
    int *selectedFeatures = (int *)checkedCalloc(noOfFeatures,sizeof(int));
    /*holds the intra feature MI values*/
    int sizeOfMatrix = k*noOfFeatures;
    double *featureMIMatrix = (double *)checkedCalloc(sizeOfMatrix,sizeof(double));

    double maxMI = 0.0;
    int maxMICounter = -1;

    /*init variables*/

    double score, currentScore, totalFeatureMI;
    int currentHighestFeature;

    int arrayPosition, i, j, x;

    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    for (i = 0; i < sizeOfMatrix; i++) {
        featureMIMatrix[i] = -1;
    }/*for featureMIMatrix - blank to -1*/


    for (i = 0; i < noOfFeatures; i++) {
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);
        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    /*************
    ** Now we have populated the classMI array, and selected the highest
    ** MI feature as the first output feature
    ** Now we move into the mRMR-D algorithm
    *************/

    for (i = 1; i < k; i++) {
        /****************************************************
        ** to ensure it selects some features
        **if this is zero then it will not pick features where the redundancy is greater than the
        **relevance
        ****************************************************/
        score = -DBL_MAX;
        currentHighestFeature = 0;
        currentScore = 0.0;
        totalFeatureMI = 0.0;

        for (j = 0; j < noOfFeatures; j++) {
            /*if we haven't selected j*/
            if (selectedFeatures[j] == 0) {
                currentScore = classMI[j];
                totalFeatureMI = 0.0;

                for (x = 0; x < i; x++) {
                    arrayPosition = x*noOfFeatures + j;
                    if (featureMIMatrix[arrayPosition] == -1) {
                        /*work out intra MI*/

                        /*double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);*/
                        featureMIMatrix[arrayPosition] = calculateMutualInformation(feature2D[(int) outputFeatures[x]], feature2D[j], noOfSamples);
                    }

                    totalFeatureMI += featureMIMatrix[arrayPosition];
                }/*for the number of already selected features*/

                currentScore -= (totalFeatureMI/i);
                if (currentScore > score) {
                    score = currentScore;
                    currentHighestFeature = j;
                }
            }/*if j is unselected*/
        }/*for number of features*/

        selectedFeatures[currentHighestFeature] = 1;
        outputFeatures[i] = currentHighestFeature;

    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(feature2D);
    FREE_FUNC(featureMIMatrix);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    feature2D = NULL;
    featureMIMatrix = NULL;
    selectedFeatures = NULL;

    for(int i=0; i<k; i++) {
        if(outputFeatures[i]>=0) outputId.push_back(outputFeatures[i]);
    }
    free(outputFeatures);
}

void FeatureSelection::CHI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputFeatures)
{
    vector<double> label(classColumn, classColumn + noOfSamples);
    vector<double> classScore;
    vector<int> classScoreIndex;
    for(int i=0; i<noOfFeatures; i++) {
        vector<double> feature(featureMatrix + i*noOfSamples, featureMatrix + (i+1)*noOfSamples);
        double score = chi2f(feature, label);
        classScore.push_back(score);
        classScoreIndex.push_back(i);
    }
    // bubble sort
    for(unsigned i=0; i<classScore.size(); i++) {
        for(unsigned j=0; j<classScore.size()-i-1; j++) {
            if(classScore[j] < classScore[j+1]) {
                swap(classScore[j], classScore[j+1]);
                swap(classScoreIndex[j], classScoreIndex[j+1]);
            }
        }
    }

    // Compute mutual info. If MI(f1,f2) > MI(f1,class), exclude it.
    vector<int> select(classScore.size(), 1); // initializing with 1 means selected
    for(unsigned i=0; i<classScore.size(); i++) {
        for(unsigned j=i+1; j<classScore.size(); j++) {
            vector<double> f1(featureMatrix + classScoreIndex[i]*noOfSamples, featureMatrix + (classScoreIndex[i]+1)*noOfSamples);
            vector<double> f2(featureMatrix + classScoreIndex[j]*noOfSamples, featureMatrix + (classScoreIndex[j]+1)*noOfSamples);
            double mi = chi2f(f1,f2);
            if(mi > classScore[j]) {
                select[j] = 0;  // excluded
            }
        }
    }
    for(unsigned i=0; i<classScore.size(); i++) {
        if(select[i]==1) {
            outputFeatures.push_back(classScoreIndex[i]);
        }
    }
}

// return the score of column data ( 966 x 1)
double FeatureSelection::chi2f(vector<double> &feature, vector<double> &label)
{
    vector<double> classL; // label's level
    for(unsigned i=0; i<feature.size(); i++) {
        if(find(classL.begin(), classL.end(), label[i])==classL.end()) { // not found, so add it
            classL.push_back(label[i]);
        }
    }
    sort(classL.begin(), classL.end());

    vector<double> member; // feature's level
    for(unsigned i=0; i<feature.size(); i++) {
        if(find(member.begin(), member.end(), feature[i])==member.end()) { // not found, so add it
            member.push_back(feature[i]);
        }
    }
    sort(member.begin(), member.end());

    vector<double> n_fstar;  // the number of member
    for(unsigned i=0; i<member.size(); i++) {
        n_fstar.push_back(count(feature.begin(), feature.end(), member[i]));
    }
    matrixColRepeat(n_fstar, classL.size());

    vector<double> ct(classL.size()*member.size(), 0); // the number of corresponding member of same index
    for(unsigned i=0; i<member.size(); i++) {
        for(unsigned j=0; j<feature.size(); j++) {
            if(feature[j]==member[i]) { // index matched with member
                for(unsigned k=0; k<classL.size(); k++) { // search for corresponding class
                    if(label[j]==classL[k]) {
                        ct[i*classL.size()+k]++;
                        break;
                    }
                }
            }
        }
    }

    vector<double> n_star;
    matrixColSum(n_star, ct, classL.size());
    matrixRowRepeat(n_star, member.size());

    vector<double> mu_i, mu_i_tmp;
    elemMul(mu_i_tmp, n_star, n_fstar);
    elemDividedByConst(mu_i, mu_i_tmp, feature.size());

    vector<double> part, part_tmp;
    elemSub(part,ct,mu_i);
    elemSquare(part_tmp,part);
    elemDiv(part, part_tmp, mu_i);

    vector<double> negPos;
    matrixRowSum(negPos,  part, member.size());

    vector<double> resultSum;
    matrixRowSum(resultSum, negPos, 1);
    double chi = resultSum[0];
    return chi;
}

void FeatureSelection::CMIM(int k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId)
{
    double *outputFeatures = new double[k];
    /*holds the class MI values
    **the class MI doubles as the partial score from the CMIM paper
    */
    double *classMI = (double *)checkedCalloc(noOfFeatures,sizeof(double));
    /*in the CMIM paper, m = lastUsedFeature*/
    int *lastUsedFeature = (int *)checkedCalloc(noOfFeatures,sizeof(int));

    double score, conditionalInfo;
    int currentFeature;

    double maxMI = 0.0;
    int maxMICounter = -1;

    int j,i;

    double **feature2D = (double**) checkedCalloc(noOfFeatures,sizeof(double*));

    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    for (i = 0; i < noOfFeatures; i++) {
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);

        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    outputFeatures[0] = maxMICounter;

    /*****************************************************************************
    ** We have populated the classMI array, and selected the highest
    ** MI feature as the first output feature
    ** Now we move into the CMIM algorithm
    *****************************************************************************/

    for (i = 1; i < k; i++) {
        score = 0.0;

        for (j = 0; j < noOfFeatures; j++) {
            while ((classMI[j] > score) && (lastUsedFeature[j] < i)) {
                /*double calculateConditionalMutualInformation(double *firstVector, double *targetVector, double *conditionVector, int vectorLength);*/
                currentFeature = (int) outputFeatures[lastUsedFeature[j]];
                conditionalInfo = calculateConditionalMutualInformation(feature2D[j],classColumn,feature2D[currentFeature],noOfSamples);
                if (classMI[j] > conditionalInfo) {
                    classMI[j] = conditionalInfo;
                }/*reset classMI*/
                /*moved due to C indexing from 0 rather than 1*/
                lastUsedFeature[j] += 1;
            }/*while partial score greater than score & not reached last feature*/
            if (classMI[j] > score) {
                score = classMI[j];
                outputFeatures[i] = j;
            }/*if partial score still greater than score*/
        }/*for number of features*/
    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(lastUsedFeature);
    FREE_FUNC(feature2D);

    classMI = NULL;
    lastUsedFeature = NULL;
    feature2D = NULL;

    for(int i=0; i<k; i++) {
        if(outputFeatures[i]>=0) outputId.push_back(outputFeatures[i]);
    }
    delete [] outputFeatures;
}/*CMIM(int,int,int,double[][],double[],double[])*/

void FeatureSelection::DISR(int k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId)
{
    double *outputFeatures = new double[k];
    /*holds the class MI values*/
    double *classMI = (double *)checkedCalloc(noOfFeatures,sizeof(double));

    char *selectedFeatures = (char *)checkedCalloc(noOfFeatures,sizeof(char));

    /*holds the intra feature MI values*/
    int sizeOfMatrix = k*noOfFeatures;
    double *featureMIMatrix = (double *)checkedCalloc(sizeOfMatrix,sizeof(double));

    double maxMI = 0.0;
    int maxMICounter = -1;

    double **feature2D = (double**) checkedCalloc(noOfFeatures,sizeof(double*));

    double score, currentScore;
    int currentHighestFeature;

    double *mergedVector = (double *) checkedCalloc(noOfSamples,sizeof(double));

    int arrayPosition;
    double mi, tripEntropy;

    int i,j,x;

    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    for (i = 0; i < sizeOfMatrix; i++) {
        featureMIMatrix[i] = -1;
    }/*for featureMIMatrix - blank to -1*/


    for (i = 0; i < noOfFeatures; i++) {
        /*calculate mutual info
        **double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);
        */
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);

        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    /*****************************************************************************
    ** We have populated the classMI array, and selected the highest
    ** MI feature as the first output feature
    ** Now we move into the DISR algorithm
    *****************************************************************************/

    for (i = 1; i < k; i++) {
        score = 0.0;
        currentHighestFeature = 0;
        currentScore = 0.0;

        for (j = 0; j < noOfFeatures; j++) {
            /*if we haven't selected j*/
            if (selectedFeatures[j] == 0) {
                currentScore = 0.0;

                for (x = 0; x < i; x++) {
                    arrayPosition = x*noOfFeatures + j;
                    if (featureMIMatrix[arrayPosition] == -1) {
                        /*
                        **double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);
                        **double calculateJointEntropy(double *firstVector, double *secondVector, int vectorLength);
                        */

                        mergeArrays(feature2D[(int) outputFeatures[x]], feature2D[j],mergedVector,noOfSamples);
                        mi = calculateMutualInformation(mergedVector, classColumn, noOfSamples);
                        tripEntropy = calculateJointEntropy(mergedVector, classColumn, noOfSamples);

                        featureMIMatrix[arrayPosition] = mi / tripEntropy;
                    }/*if not already known*/
                    currentScore += featureMIMatrix[arrayPosition];
                }/*for the number of already selected features*/

                if (currentScore > score) {
                    score = currentScore;
                    currentHighestFeature = j;
                }
            }/*if j is unselected*/
        }/*for number of features*/

        selectedFeatures[currentHighestFeature] = 1;
        outputFeatures[i] = currentHighestFeature;

    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(mergedVector);
    FREE_FUNC(feature2D);
    FREE_FUNC(featureMIMatrix);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    mergedVector = NULL;
    feature2D = NULL;
    featureMIMatrix = NULL;
    selectedFeatures = NULL;

    for(int i=0; i<k; i++) {
        if(outputFeatures[i]>=0) outputId.push_back(outputFeatures[i]);
    }
    delete [] outputFeatures;
}/*DISR(int,int,int,double[][],double[],double[])*/

void FeatureSelection::CondMI(int k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId)
{
    double *outputFeatures = new double[k];

    /*holds the class MI values*/
    double *classMI = (double *)checkedCalloc(noOfFeatures,sizeof(double));

    char *selectedFeatures = (char *)checkedCalloc(noOfFeatures,sizeof(char));

    /*holds the intra feature MI values*/
    int sizeOfMatrix = k*noOfFeatures;
    double *featureMIMatrix = (double *)checkedCalloc(sizeOfMatrix,sizeof(double));

    double maxMI = 0.0;
    int maxMICounter = -1;

    double **feature2D = (double**) checkedCalloc(noOfFeatures,sizeof(double*));

    double score, currentScore;
    int currentHighestFeature;

    double *conditionVector = (double *) checkedCalloc(noOfSamples,sizeof(double));

    int i,j;

    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    for (i = 0; i < sizeOfMatrix; i++) {
        featureMIMatrix[i] = -1;
    }/*for featureMIMatrix - blank to -1*/

    for (i = 0; i < noOfFeatures; i++) {
        /*calculate mutual info
        **double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);
        */
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);

        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    memcpy(conditionVector,feature2D[maxMICounter],sizeof(double)*noOfSamples);

    /*****************************************************************************
    ** We have populated the classMI array, and selected the highest
    ** MI feature as the first output feature
    ** Now we move into the CondMI algorithm
    *****************************************************************************/

    for (i = 1; i < k; i++) {
        score = 0.0;
        currentHighestFeature = -1;
        currentScore = 0.0;

        for (j = 0; j < noOfFeatures; j++) {
            /*if we haven't selected j*/
            if (selectedFeatures[j] == 0) {
                currentScore = 0.0;

                /*double calculateConditionalMutualInformation(double *firstVector, double *targetVector, double *conditionVector, int vectorLength);*/
                currentScore = calculateConditionalMutualInformation(feature2D[j],classColumn,conditionVector,noOfSamples);

                if (currentScore > score) {
                    score = currentScore;
                    currentHighestFeature = j;
                }
            }/*if j is unselected*/
        }/*for number of features*/

        outputFeatures[i] = currentHighestFeature;

        if (currentHighestFeature != -1) {
            selectedFeatures[currentHighestFeature] = 1;
            mergeArrays(feature2D[currentHighestFeature],conditionVector,conditionVector,noOfSamples);
        }

    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(conditionVector);
    FREE_FUNC(feature2D);
    FREE_FUNC(featureMIMatrix);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    conditionVector = NULL;
    feature2D = NULL;
    featureMIMatrix = NULL;
    selectedFeatures = NULL;

    for(int i=0; i<k; i++) {
        if(outputFeatures[i]>=0) outputId.push_back(outputFeatures[i]);
    }
    delete [] outputFeatures;
}/*CondMI(int,int,int,double[][],double[],double[])*/

void FeatureSelection::ICAP(int k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId)
{
    double *outputFeatures = new double[k];

    /*holds the class MI values*/
    double *classMI = (double *)checkedCalloc(noOfFeatures,sizeof(double));
    char *selectedFeatures = (char *)checkedCalloc(noOfFeatures,sizeof(char));

    /*separates out the features*/
    double **feature2D = (double **) checkedCalloc(noOfFeatures,sizeof(double *));

    /*holds the intra feature MI values*/
    int sizeOfMatrix = k*noOfFeatures;
    double *featureMIMatrix = (double *)checkedCalloc(sizeOfMatrix,sizeof(double));
    double *featureCMIMatrix = (double *)checkedCalloc(sizeOfMatrix,sizeof(double));

    double maxMI = 0.0;
    int maxMICounter = -1;

    double score, currentScore, totalFeatureInteraction, interactionInfo;
    int currentHighestFeature, arrayPosition;

    int i, j, m;

    for (j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int) j * noOfSamples;
    }

    for (i = 0; i < sizeOfMatrix; i++) {
        featureMIMatrix[i] = -1;
        featureCMIMatrix[i] = -1;
    }/*for featureMIMatrix and featureCMIMatrix - blank to -1*/

    /*SETUP COMPLETE*/
    /*Algorithm starts here*/

    for (i = 0; i < noOfFeatures; i++) {
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);

        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    /*************
     ** Now we have populated the classMI array, and selected the highest
     ** MI feature as the first output feature
     *************/

    for (i = 1; i < k; i++) {
        /**********************************************************************
        ** to ensure it selects some features
        **if this is zero then it will not pick features where the redundancy is greater than the
        **relevance
        **********************************************************************/
        score = -DBL_MAX;
        currentHighestFeature = 0;
        currentScore = 0.0;

        for (j = 0; j < noOfFeatures; j++) {
            /*if we haven't selected j*/
            if (!selectedFeatures[j]) {
                currentScore = classMI[j];
                totalFeatureInteraction = 0.0;

                for (m = 0; m < i; m++) {
                    arrayPosition = m*noOfFeatures + j;

                    if (featureMIMatrix[arrayPosition] == -1) {
                        /*work out interaction*/

                        /*double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);*/
                        featureMIMatrix[arrayPosition] = calculateMutualInformation(feature2D[(int) outputFeatures[m]], feature2D[j], noOfSamples);
                        /*double calculateConditionalMutualInformation(double *firstVector, double *targetVector, double* conditionVector, int vectorLength);*/
                        featureCMIMatrix[arrayPosition] = calculateConditionalMutualInformation(feature2D[(int) outputFeatures[m]], feature2D[j], classColumn, noOfSamples);
                    }/*if not already known*/

                    interactionInfo = featureCMIMatrix[arrayPosition] - featureMIMatrix[arrayPosition];

                    if (interactionInfo < 0) {
                        totalFeatureInteraction += interactionInfo;
                    }
                }/*for the number of already selected features*/

                currentScore += totalFeatureInteraction;


                if (currentScore > score) {
                    score = currentScore;
                    currentHighestFeature = j;
                }
            }/*if j is unselected*/
        }/*for number of features*/

        selectedFeatures[currentHighestFeature] = 1;
        outputFeatures[i] = currentHighestFeature;

    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(feature2D);
    FREE_FUNC(featureMIMatrix);
    FREE_FUNC(featureCMIMatrix);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    feature2D = NULL;
    featureMIMatrix = NULL;
    featureCMIMatrix = NULL;
    selectedFeatures = NULL;

    for(int i=0; i<k; i++) {
        if(outputFeatures[i]>=0) outputId.push_back(outputFeatures[i]);
    }
    delete [] outputFeatures;
}/*ICAP(int,int,int,double[][],double[],double[])*/

void FeatureSelection::MIM(int k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId)
{
    double *outputFeatures = new double[k];
    double **feature2D = (double **) checkedCalloc(noOfFeatures,sizeof(double *));

    /*holds the class MI values*/
    double *classMI = (double *)checkedCalloc(noOfFeatures,sizeof(double));
    char *selectedFeatures = (char *)checkedCalloc(noOfFeatures,sizeof(char));
    double maxMI = 0.0;
    int maxMICounter = -1;
    int i,j;

    /**********************************************************
    ** this pulls out a pointer to the first sample of
    ** each feature and stores it as a multidimensional array
    ** so it can be indexed nicely
    ***********************************************************/
    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    /***********************************************************
    ** SETUP COMPLETE
    ** Algorithm starts here
    ***********************************************************/

    for (i = 0; i < noOfFeatures; i++) {
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);

        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    /*************
    ** Now we have populated the classMI array, and selected the highest
    ** MI feature as the first output feature.
    *************/

    /**
     * Ideally this should use a quick sort, but it's still quicker than
     * calling BetaGamma with beta=0 and gamma=0
     */
    for (i = 1; i < k; i++) {
        maxMI = -1.0;
        for (j = 0; j < noOfFeatures; j++) {
            if (!selectedFeatures[j]) {
                if (maxMI < classMI[j]) {
                    maxMI = classMI[j];
                    maxMICounter = j;
                }
            }
        }
        selectedFeatures[maxMICounter] = 1;
        outputFeatures[i] = maxMICounter;
    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(feature2D);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    feature2D = NULL;
    selectedFeatures = NULL;

    for(int i=0; i<k; i++) {
        if(outputFeatures[i]>=0) outputId.push_back(outputFeatures[i]);
    }
    delete [] outputFeatures;
}/*MIM(int,int,int,double[][],double[],double[])*/


void FeatureSelection::FCBF(int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, double threshold, vector<int> &outputId)
{
    vector<double> classScore;
    vector<int> classScoreIndex;
    for(int i=0; i<noOfFeatures; i++) {
        double score = SU(featureMatrix + i*noOfSamples, classColumn, noOfSamples);
        classScore.push_back(score);
        classScoreIndex.push_back(i);
    }
    // bubble sort (descent)
    for(unsigned i=0; i<classScore.size(); i++) {
        for(unsigned j=0; j<classScore.size()-i-1; j++) {
            if(classScore[j] < classScore[j+1]) {
                swap(classScore[j], classScore[j+1]);
                swap(classScoreIndex[j], classScoreIndex[j+1]);
            }
        }
    }
    // find threshold
    int thresholdPos = -1;
    for(unsigned i=0; i<classScore.size(); i++) {
        if(classScore[i]<threshold) {
            thresholdPos = i;
            break;
        }
    }

    // Compute mutual info. If MI(f1,f2) > MI(f1,class), exclude it.
    vector<int> select(classScore.size(), 1); // initializing with 1 means selected
    for(int i=0; i<thresholdPos; i++) {
        for(int j=i+1; j<thresholdPos; j++) {
            double score = SU(featureMatrix + classScoreIndex[i]*noOfSamples, featureMatrix + classScoreIndex[j]*noOfSamples, noOfSamples);
            if(score > classScore[j]) {
                select[j] = 0;  // excluded
            }
        }
    }

    for(int i=0; i<thresholdPos; i++) {
        if(select[i]==1) {
            outputId.push_back(classScoreIndex[i]);
        }
    }
}

double FeatureSelection::SU(double *dataVector1, double *dataVector2, int vectorLength)
{
    double hX = calculateEntropy(dataVector1, vectorLength);
    double hY = calculateEntropy(dataVector2, vectorLength);
    double iXY = calculateMutualInformation(dataVector1, dataVector2, vectorLength);
    double score = (2 * iXY) / (hX + hY);
    return score;
}
