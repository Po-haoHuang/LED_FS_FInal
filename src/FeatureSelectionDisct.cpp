#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>

#include "../include/FeatureSelection.h"

using std::cout;
using std::endl;
using std::vector;

bool FeatureSelection::disct_ew(vector<vector<double> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr)
{
    vector<vector<double> > *inFeatureDataPtr;
    if(inDataPtr==NULL){
        inFeatureDataPtr = &(this->featureData);
        #ifdef DEBUG_INFO
        cout << "Start disct_ew ... ";
        #endif
    }else{
        inFeatureDataPtr = inDataPtr;
    }
    vector<vector<double> > &inFeatureData = *inFeatureDataPtr;

    if(partitionNum<=1){
        vector<double> dataLine(numOfFeatures(),1);
        for(unsigned i=0; i<inFeatureData.size(); i++){
            discreteData.push_back(dataLine);
        }
        #ifdef DEBUG_INFO
        cout << "done." << endl;
        #endif
        return true;
    }

    vector<double> attrMax(numOfFeatures(),-DBL_MAX);
    vector<double> attrMin(numOfFeatures(),DBL_MAX);
    vector<double> interval(numOfFeatures());
    for(unsigned i=0; i<inFeatureData.size(); i++){
        for(unsigned j=0; j<numOfFeatures(); j++){
            if(inFeatureData[i][j] > attrMax[j])
                attrMax[j] = inFeatureData[i][j];
            if(inFeatureData[i][j] < attrMin[j])
                attrMin[j] = inFeatureData[i][j];
        }
    }

    for(unsigned i=0; i<numOfFeatures(); i++){
        interval[i] = (attrMax[i] - attrMin[i])/partitionNum;
    }

    vector<double> discreteLineValue;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        discreteLineValue.clear();
        for(unsigned j=0; j<numOfFeatures(); j++){
            int level;
            if(interval[j]==0){
                level = 1;
            }else{
                level = (inFeatureData[i][j]-attrMin[j])/interval[j] + 1.0;
            }
            if(level>partitionNum) level = partitionNum;
            else if(level<1) level = 1;
            discreteLineValue.push_back(level);
        }
        discreteData.push_back(discreteLineValue);
    }
    if(inDataPtr==NULL){
        // only output the selected features
        vector<vector<double> > selectedData;
        for(unsigned row=0; row<numOfSamples(); row++){
            vector<double> oneSample;
            for(unsigned col=0; col<numOfUsedFeatures(); col++){
                oneSample.push_back(discreteData[row][useFeatureId_[col]]);
            }
            selectedData.push_back(oneSample);
        }
        selectedData.swap(discreteData);
        #ifdef DEBUG_INFO
        cout << "done." << endl;
        #endif
    }
    return true;
}

bool FeatureSelection::disct_ew_cycle(vector<vector<double> >& discreteData, int partitionNum)
{
    #ifdef DEBUG_INFO
    cout << "Start cycle_disct_ew ... containing cycle: ";
    #endif
    vector<vector<double> > cycleFeatureData;
    vector<vector<double> > cycleDiscreteData;
    for(unsigned i=0; i<featureData.size(); i++){
        cycleFeatureData.push_back(featureData[i]);
        // when reading last element or cycles not equal, it's the end of cycleDiscreteData
        if(i==featureData.size()-1 || featureDataCycle[i]!=featureDataCycle[i+1]){
            #ifdef DEBUG_INFO
            cout << featureDataCycle[i] << " ";
            #endif
            disct_ew(cycleDiscreteData, partitionNum, &cycleFeatureData);
            for(unsigned ci=0; ci<cycleDiscreteData.size(); ci++){
                discreteData.push_back(cycleDiscreteData[ci]);
            }
            cycleFeatureData.clear();
            cycleDiscreteData.clear();
        }
    }
    // only output the selected features
    vector<vector<double> > selectedData;
    for(unsigned row=0; row<numOfSamples(); row++){
        vector<double> oneSample;
        for(unsigned col=0; col<numOfUsedFeatures(); col++){
            oneSample.push_back(discreteData[row][useFeatureId_[col]]);
        }
        selectedData.push_back(oneSample);
    }
    selectedData.swap(discreteData);
    #ifdef DEBUG_INFO
    cout << "done." << endl;
    #endif
    return true;
}

bool FeatureSelection::disct_col_manual(vector<double> &inFeatureData, vector<double> &discreteData, vector<double> &cutPoints, bool increaseNoBack)
{
    if(cutPoints.size()<=1){  // return n*1 matrix
        for(unsigned i=0; i<inFeatureData.size(); i++){
            discreteData.push_back(1);
        }
        return true;
    }
    sort(cutPoints.begin(), cutPoints.end());
    int thresholdLevel = 1;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        int level=-1;
        double currentValue = inFeatureData[i];
        // determine current value level
        for(unsigned j=0; j<cutPoints.size(); j++){
            if(currentValue < cutPoints[j]){
                level = j+1;
                break;
            }
        }
        if(level==-1){  // level undetermined, larger than last cut point
            level = cutPoints.size()+1;  // assign to max level
        }
        if(level > thresholdLevel){
            thresholdLevel = level;
        }
        if(increaseNoBack)
            discreteData.push_back(thresholdLevel);
        else
            discreteData.push_back(level);

        // when reading last element or cycles not equal, it's the end of cycleDiscreteData
        if(i==featureData.size()-1 || featureDataCycle[i]!=featureDataCycle[i+1]){
            thresholdLevel = 1;  // reset for next cycle
        }
    }
    return true;
}

void FeatureSelection::disct_col_ew_cycle(vector<double> &inFeatureData, vector<double> &outDisctData, int partitionNum, bool increaseNoBack)
{
    vector<double> cycleFeatureData;
    vector<double> cycleDiscreteData;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        cycleFeatureData.push_back(inFeatureData[i]);
        // when reading last element or cycles not equal, it's the end of cycleDiscreteData
        if(i==inFeatureData.size()-1 || featureDataCycle[i]!=featureDataCycle[i+1]){
            double attrMax = -DBL_MAX;
            double attrMin = DBL_MAX;
            for(unsigned i=0; i<cycleFeatureData.size(); i++){
                if(cycleFeatureData[i] > attrMax)
                    attrMax = cycleFeatureData[i];
                if(cycleFeatureData[i] < attrMin)
                    attrMin = cycleFeatureData[i];
            }
            double interval = (attrMax - attrMin)/partitionNum;
            vector<double> cutPoints;
            sout << "cycle " << featureDataCycle[i] << " cut points:,";
            for(int i=1; i<partitionNum; i++){
                sout << attrMin + i*interval << ",";
                cutPoints.push_back(attrMin + i*interval);
            }
            sout << endl;
            vector<double> cycleDisctData;
            disct_col_manual(cycleFeatureData, cycleDisctData, cutPoints, increaseNoBack);

            for(unsigned ci=0; ci<cycleDisctData.size(); ci++){
                outDisctData.push_back(cycleDisctData[ci]);
            }
            cycleFeatureData.clear();
            cycleDiscreteData.clear();
        }
    }
}
