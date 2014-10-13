#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
//gsl library for statistic
#include"gsl/gsl_sort.h"
#include"gsl/gsl_statistics_double.h"
#include<iostream>
#include<vector>
#include<string>
#include <fstream>
#include <dirent.h>


//include file processing part
#include "../include/FileData.h"
#include "../include/DataBase.h"
#include "../include/CycleData.h"

//Variable for feature extraction
unsigned attrNum;
unsigned dataSize;
unsigned featureNum = 10;
string featureList = "0123456789";
unsigned fileNum;
unsigned segNum;
unsigned timeLowerBound;
vector<FileData*> fileDataVector;
vector<vector<double> > rowData;

using namespace std;

//Function declaration
//For FE
void runFeatureExtraction(DataBase db,char* cycleBegin,char* cycleEnd);
void FeatureExtraction(unsigned chunkSize,double* cleanData,vector<double>& output);



//Feature_name list
const char featureName[][30] ={"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};

//type for segmentation 1:no segmentation  2:segmentation
typedef enum {disable, enable}segmentPara;
segmentPara seg = disable;


int main(int argc, char *argv[]){
	string cycleListFileName,dataDir;
	int cycleBegin,cycleEnd;
	

	//checking argv input
	if(argc > 8){//too many arguments
		cerr<<"Too many arguments.\n";
		system("pause");
		return -1;
	}

	else if(argc < 8){//too few arguments
		cerr<<"Too few arguments.\n";
		system("pause");
		return -1;
	}
	else{
	//data DIR
	cycleListFileName = argv[1];
	dataDir = argv[2];
	//Set cycle range
	if(atoi(argv[3])!='\0')
	cycleBegin = atoi(argv[3]);
	else{
	cerr << "Cyclr brgin number error!";
	system("pause");
	return 1;
	}
	if(atoi(argv[4])!='\0')
	cycleEnd = atoi(argv[4]);
	else{
	cerr << "Cycle end number error!";
	system("pause");
	return 1;
	}
	//set Segmentation number 
	segNum = atoi(argv[5]);
	if (segNum == 1)
		seg = disable;
	else 
		seg = enable;
	}
	timeLowerBound = atoi(argv[6]);
	featureList = argv[7];
	featureNum = string(argv[7]).size();

	//Start File IO	
 	// database
    DataBase db;

    db.init(dataDir, cycleListFileName);   // use filtered files directory as working directory

    if(!db.valid()){
        cerr << "Database initializing failed." << endl;
        system("pause");
        return 1;
    }
    cout<<endl;
    runFeatureExtraction(db,argv[3],argv[4]);
	return 0;
}

void runFeatureExtraction(DataBase db,char* cycleBegin,char* cycleEnd){
	
	//Initialization
	vector<vector<double> > singleResult;//Result for a single file
	vector<double> tempResult;//Result for a single attribute(variable)
	vector<double>temp;//All data in a single file for an attribute
	double* temp_array;//Turn temp into array version for computing(actually only pass memory address)
	FileData tempfd;
	db.extractById(db.beginOfCycle(atoi(cycleBegin)),tempfd);
   	attrNum = tempfd.attrSize();


	//enable or disable
	switch (seg){
		case disable:{//no segmentation
			
			//Initialization
			string tempFile;
			tempFile += "Output_noSeg_C";
			tempFile += cycleBegin;
			tempFile +="-C";
			tempFile += cycleEnd;
			tempFile += ".csv";

			FILE* fout = fopen(tempFile.c_str(),"w+");

			fprintf(fout,"%s,%s,%s","Id","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout,",%s_%s",tempfd.attrTypeVector[j+1].c_str(),featureName[i]);
				}
			}

			
			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first file
			while first call FE.
			Result:File_1_Attr_1_firstfeature - File_1_Attr_1_lasttfeature*/
			unsigned fileCount = 1;
			for(unsigned cycleNum = (unsigned)atoi(cycleBegin);
				cycleNum <= (unsigned)atoi(cycleEnd);cycleNum++){
				for(unsigned id=db.beginOfCycle(cycleNum);
					id<db.endOfCycle(cycleNum); id++){
					//Initialization for a new file
					FileData fd;
        			if(db.extractById(id,fd)&&fd.dataVector.size() >= timeLowerBound){
        				//start FE
        				fd.id = fileCount;
        				fileCount++;
						rowData = fd.dataVector;
						dataSize = fd.dataVector.size();
						temp.clear();
						temp.resize(dataSize);
						singleResult.clear();
						
						singleResult.resize(featureNum);
						for(unsigned j = 0;j < attrNum;j++){
							for(unsigned i = 0;i < dataSize;i++)
								temp[i] = rowData[i][j];
	 						temp_array = &temp[0];
	 						tempResult.clear();
							FeatureExtraction(dataSize,temp_array,tempResult);
							for(unsigned i = 0;i < featureNum;i++){
								singleResult[i].push_back(tempResult[i]);
							}
						}
						//output

						fprintf(fout,"\n");

						fprintf(fout,"%d,%d,%d",fd.id,fd.fid,fd.nCycle);
						for(unsigned j = 0;j < attrNum;j++){
							for(unsigned i = 0;i < featureNum;i++){
								fprintf(fout,",%lf",singleResult[i][j]);
							}
						}
						if( (fileCount)!=0 && ((fileCount)%100)==0 )
							cout<<"Computing "<<fileCount<<" files."<<endl;
					}
				}
			}	

			cout<<"Computing done."<<endl;
			cout<<"Output processing..."<<endl;
			cout<<"Output done."<<endl;
			fclose(fout);
		}
			break;
			
		case enable:{//segmentation enabled

			//Initialization
			vector<double > originalDP;
			unsigned tempSize = 0;
			unsigned dpID = 0;
			string f1,f2;
			char itoatemp[50] ;
			f1 = "Output_seg";
			itoa(segNum,itoatemp,10);
			f1 += itoatemp;
			f1 += "_";
			f2 = f1;
			f2 += "2_C";
			f1 += "1_C";
			f2 += cycleBegin;
			f2 += "-C";
			f2 += cycleEnd;
			f1 += cycleBegin;
			f1 += "-C";
			f1 += cycleEnd;
			f2 += ".csv";
			f1 += ".csv";
			FILE* fout1 = fopen(f1.c_str(),"w+");
			FILE* fout2 = fopen(f2.c_str(),"w+");
			
			//Pre-Output
			fprintf(fout1,"%s,%s,%s","Id","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				if(tempfd.attrTypeVector[j+1].find("dP_Filter") != string::npos){
					dpID = j;
					for(unsigned i = 0;i < featureNum;i++)
						fprintf(fout1,",%s_%s",tempfd.attrTypeVector[j+1].c_str(),featureName[i]);
					continue;
				}
				for(unsigned i = 0;i < featureNum;i++){
                    for(unsigned k = 0;k < segNum;k++){
						fprintf(fout1,",%s%s%d_%s",tempfd.attrTypeVector[j+1].c_str(),"_Seg_",k,featureName[i]);
					}
				}
			}

			fprintf(fout2,"%s,%s,%s","Id","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout2,",%s_%s",tempfd.attrTypeVector[j+1].c_str(),featureName[i]);
				}
			}


			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first segment
			of file while first call FE.
			Result:File_1_Seg_1_Attr_1_firstfeature - File_1_Seg_1_Attr_1_lasttfeature*/
			unsigned fileCount = 1;
			for(unsigned cycleNum = (unsigned)atoi(cycleBegin);
				cycleNum <= (unsigned)atoi(cycleEnd);cycleNum++){
				for(unsigned id=db.beginOfCycle(cycleNum);
					id<db.endOfCycle(cycleNum); id++){
					FileData fd;
					if(db.extractById(id,fd)&&fd.dataVector.size() >= timeLowerBound){
						//Initialization for a new file
        				fd.id = fileCount;
        				fileCount++;						
						rowData = fd.dataVector;
						dataSize = fd.dataVector.size();
						singleResult.clear();
						singleResult.resize(featureNum*segNum);

						temp.clear();
						temp.resize(dataSize);
					
						//Computing non-segmented DP_filter
						originalDP.clear();
						originalDP.resize(featureNum);


						for(unsigned i = 0;i < dataSize;i++)
							temp[i] = rowData[i][dpID];
						temp_array = &temp[0];
						tempResult.clear();
						FeatureExtraction(dataSize,temp_array,tempResult);
						for(unsigned i = 0;i < featureNum;i++){
							originalDP[i] = tempResult[i];
						}


						for (unsigned l = 0;l < segNum;l++){
							for(unsigned j = 0;j < attrNum;j++){

								tempSize = 0;
								temp.clear();
								if(l != segNum - 1){
									for(unsigned i = round((float)dataSize/(float)segNum*(float)l);
									i <= round((float)dataSize/(float)segNum*(float)(l+1)-1.0);i++){
										tempSize++;
										temp.push_back(rowData[i][j]);
									}
								}
							
								else{
									for(unsigned i =  round((float)dataSize/(float)segNum*(float)l);
									i < dataSize;i++){
										tempSize++;
										temp.push_back(rowData[i][j]);
									}
								}

								temp_array = &temp[0];
								tempResult.clear();
								FeatureExtraction(tempSize,temp_array,tempResult);
								for(unsigned i = 0+l*featureNum;i < 0+l*featureNum+featureNum;i++){
									singleResult[i].push_back(tempResult[i-l*featureNum]);
								}
							}
						}
						fprintf(fout1,"\n");
						fprintf(fout1,"%d,%d,%d",fd.id,
						fd.fid,fd.nCycle);

						for(unsigned j = 0;j < attrNum;j++){
							for(unsigned i = 0;i < featureNum;i++){
								if(j == dpID){
									fprintf(fout1,",%lf",originalDP[i]);
									continue;
								}
								for(unsigned l = 0;l < segNum;l++)
									fprintf(fout1,",%lf",singleResult[l*featureNum+i][j]);
							}
						}
					
						for(unsigned l = 0;l < segNum;l++){
							fprintf(fout2,"\n");
							fprintf(fout2,"%d,%d,%d",fd.id,
							fd.fid,fd.nCycle);
							for(unsigned j = 0;j < attrNum;j++){
								for(unsigned i = 0;i < featureNum;i++){
									fprintf(fout2,",%lf",singleResult[l*featureNum+i][j]);
								}
							}
						}
						if( (fileCount)!=0 && ((fileCount)%100)==0 )
							cout<<"Computing "<<fileCount<<" files."<<endl;
					}
				}
			}
			cout<<"Computing done."<<endl;
			cout<<"Output processing..."<<endl;
			fclose(fout1);
			fclose(fout2);
			cout<<"Output done."<<endl;
		}
		break;
	}
}

void FeatureExtraction(unsigned chunkSize,double* cleanData,vector<double>& output){
	
	unsigned length = chunkSize;
	//static double f[featureNum];
	double tempData[chunkSize];
	
	
	//tempData = cleanData^2;
	for(unsigned i = 0;i < length;i++)
		tempData[i] = pow(cleanData[i], 2);

	//three para for gsl func (data_array,element size(#double),#elements)
	if(featureList.find("0") != string::npos)
		output.push_back(gsl_stats_mean (cleanData, 1, length));
	if(featureList.find("1") != string::npos)
		output.push_back(gsl_stats_variance (cleanData, 1, length));
	if(featureList.find("2") != string::npos)
		output.push_back(gsl_stats_skew (cleanData, 1, length));
	if(featureList.find("3") != string::npos)
		output.push_back(gsl_stats_kurtosis (cleanData, 1, length));
	if(featureList.find("4") != string::npos)
		output.push_back(gsl_stats_max (cleanData, 1, length));
	if(featureList.find("5") != string::npos)
		output.push_back(gsl_stats_min (cleanData, 1, length));
	if(featureList.find("6") != string::npos)
		output.push_back(sqrt (gsl_stats_mean (tempData, 1, length)));//rms
 	if(featureList.find("7") != string::npos)
		output.push_back(gsl_stats_sd (cleanData, 1, length));//std
	if(featureList.find("8") != string::npos)
		output.push_back(gsl_stats_max (cleanData, 1, length)-gsl_stats_min (cleanData, 1, length));//range
	if(featureList.find("9") != string::npos){
		gsl_sort (cleanData, 1, length);//sort before iqr
		output.push_back(gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
			- gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25));//iqr
	}

	

	for(unsigned i = 0;i < featureNum;i++){//set 0 if nan
		if(isnan(output[i]))
			output[i] = 0;
	}
	return;
	
}

