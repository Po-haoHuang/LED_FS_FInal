#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<vector>
#include<string>

bool FeatureSetFilter(vector<vector<double> > TrainFeatureData,vector<string> ParaFeatureList,vector<int> LifeCycleInd,int type,string fname,string vname,string feature){
	string filterFeature;
	vector<int > filterFeatureInd;
	if(type == 1){
		filterFeature = strcat(vname.c_str(),"-");
		filterFeature += feature.c_str();
	}
	
	else
		filterFeature = vname.c_str();
		
	for(int i = 0;i < ParaFeatureList.size();i++){
		if(strcmp(ParaFeatureList[i].c_str(),filterFeature) == 0){
			filterFeatureInd.push_back(i);
		}
	
	}
	
		
	

	return 0;
}





