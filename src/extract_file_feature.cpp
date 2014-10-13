#include "../include/csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//gsl library for statistic
#include "gsl/gsl_sort.h"
#include "gsl/gsl_statistics_double.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <dirent.h>
#include <sstream>

//Variable for feature extraction
unsigned attrNum;
unsigned dataSize;
const unsigned featureNum = 10;
using namespace std;

//Function declaration


//Feature_name list
const char featureName[][30] ={"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};
const char* selectedFeaturesFile = "FSo_SelectedData.csv";

void csvValueSplit(string s, const char delimiter, vector<double> &lineValue)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    while (end <= string::npos){
        lineValue.push_back(atof((s.substr(start, end-start)).c_str()));
	    if (end == string::npos)
	    	break;
    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
}

void csvStrSplit(string s, const char delimiter, vector<string> &value)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    while (end <= string::npos){
        string str;
        if(s[0]=='"'){
            str = s.substr(start+1, end-start-2);
        }else{
            str = s.substr(start, end-start);
        }
        value.push_back(str);
        if (end == string::npos)
	    	break;
    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
}

bool extract_file_feature(string fileName, vector<string> usedFeatureName, vector<vector<double> > &matrix)
{
    // open file
    ifstream inFile(fileName.c_str(), ios::in);
    if(!inFile) {
        cout << "Error! Cannot open file: " << fileName << endl;
        return false;
    }
    string lineBuffer;
    vector<string> title;

    // read the first line (title)
    getline(inFile, lineBuffer);
    csvStrSplit(lineBuffer, ',', title);

    // locate feature index
    vector<int> featureIndex;
    for(unsigned i=0; i<usedFeatureName.size(); i++){
        for(unsigned j=0; j<title.size(); j++){
            if(usedFeatureName[i] == title[j]){
                featureIndex.push_back(j);
                break;
            }
        }
    }

    // read data by line, 2 method implement
    #ifndef USE_FAST_CSV
    while(getline(inFile, lineBuffer)){
        vector<double> lineValue, lineValueP;
        csvValueSplit(lineBuffer, ',', lineValue);
        for(unsigned i=0; i<featureIndex.size(); i++){
            lineValueP.push_back(lineValue[featureIndex[i]]);
        }
        matrix.push_back(lineValueP);
    }
    #endif
    inFile.close();

    #ifdef USE_FAST_CSV
    csvIO::LineReader in(fileName);
    in.next_line(); // discard attribute title
    char *linePtr;
    while( linePtr = in.next_line()){
        vector<double> lineValue, lineValueP;
        csvValueSplit(string(linePtr), ',', lineValue);
        for(unsigned i=0; i<featureIndex.size(); i++){
            lineValueP.push_back(lineValue[featureIndex[i]]);
        }
        matrix.push_back(lineValueP);
    }
    #endif

    // matrix transpose to column-major
    vector<vector<double> > newMatrix;
    for(unsigned type=0; type<matrix[0].size(); type++){
        vector<double> newColumn;
        for(unsigned sample=0; sample<matrix.size(); sample++){
            newColumn.push_back(matrix[sample][type]);
        }
        newMatrix.push_back(newColumn);
    }
    matrix = newMatrix;
    return true;
}

double FeatureExtraction(unsigned chunkSize,double* cleanData,unsigned calculate){

	unsigned length = chunkSize;
	static double f;
	double tempData[chunkSize];


	//tempData = cleanData^2;
	for(unsigned i = 0;i < length;i++)
		tempData[i] = pow(cleanData[i], 2);

	//three para for gsl func (data_array,element size(#double),#elements)
    switch(calculate){
        case 0:{
            f = gsl_stats_mean (cleanData, 1, length);
            break;
        }
        case 1:{
            f = gsl_stats_variance (cleanData, 1, length);
            break;
        }
        case 2:{
            f = gsl_stats_skew (cleanData, 1, length);
            break;
        }
        case 3:{
            f = gsl_stats_kurtosis (cleanData, 1, length);
            break;
        }
        case 4:{
            f = sqrt (gsl_stats_mean (tempData, 1, length));//rms
            break;
        }
        case 5:{
            f = gsl_stats_max (cleanData, 1, length);
            break;
        }
        case 6:{
            f = gsl_stats_min (cleanData, 1, length);
            break;
        }
        case 7:{
            f = gsl_stats_max (cleanData, 1, length) -
            gsl_stats_min (cleanData, 1, length);;//range
            break;
        }
        case 8:{
            gsl_sort (cleanData, 1, length);//sort before iqr
            //iqr
            f = gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
                - gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25);
            break;
        }

        case 9:{
            f = gsl_stats_sd (cleanData, 1, length);//std
            break;
        }
    }
    //set 0 if nan
    if(isnan(f))
		f = 0;
	return f;

}

int main(int argc, char *argv[]){
	string inputFileName;
	vector<string > selectedFeatures,calculateFeatures;


	//checking argv input
	if(argc > 2){//too many arguments
		cerr<<"Too many arguments.\n";
		system("pause");
		return -1;
	}

	else if(argc < 2){//too few arguments
		cerr<<"Too few arguments.\n";
		system("pause");
		return -1;
	}
	else{
	//input file name
	inputFileName = (string)argv[1];
	}

    //read title from FS output
    string tempAttr,tempName;
    ifstream is(selectedFeaturesFile,ios::in);
    if(!is) {
        cerr << "Error! Cannot open file: " << selectedFeaturesFile << endl;
        return false;
    }
    getline(is,tempAttr);
    stringstream temp(tempAttr);
    //get title recursively
    while(1){

        getline(temp,tempName,',');
        //get title
		selectedFeatures.push_back(strtok((char*)tempName.c_str(),"_"));
		//get calculate feture name (Ex: mean,max and etc.,)
		calculateFeatures.push_back(strtok(NULL,"\n"));
        if(temp.eof())
            break;
    }

    vector<vector<double> > matrix;
    vector<double> Output;
    //extract from online new input
    extract_file_feature(inputFileName, selectedFeatures, matrix);
    //calculating features
	for(unsigned i = 0;i < (unsigned)calculateFeatures.size();i++){
		for(unsigned j = 0;j < featureNum;j++){
			if(calculateFeatures[i].find(featureName[j]) != string::npos){
                Output.push_back(FeatureExtraction(matrix[i].size(),&matrix[i][0],j));
			}
		}

	}
    //output format
    FILE *fout;
    fout = fopen("Online Feature output.csv","w+");
    for(unsigned i = 0;i < selectedFeatures.size();i++){
        if (i != 0)
            fprintf(fout,",");
        fprintf(fout,"%s_%s",selectedFeatures[i].c_str(),calculateFeatures[i].c_str());
    }
        fprintf(fout,"\n");
    for (unsigned i = 0;i < selectedFeatures.size();i++){
        if(i != 0)
            fprintf(fout,",");
        fprintf(fout,"%lf",Output[i]);

    }
    fclose(fout);
	for(unsigned i = 0;i < Output.size();i++){
        cout<<Output[i]<<endl;
	}
	return 0;
}
