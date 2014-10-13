#include "../include/Regression.h"
#include"../include/LinearRegression.h"
#include"../include/Lars.h"
Regression::Regression(){

}

Regression::~Regression(){

}
bool Regression::init(vector<vector<double > > input,
					  vector<double > response,
					  vector<vector<double > >& normal,
					  vector<double>& outputMean,
					  vector<double>& outputStd){

	//convert vector to vec
	response_mod = vec(response);
	//vector<vector> to mat make it 1-D vector
	//and initialize a mat, reshape to 2-D
	vector<double> temp;
	temp.reserve(input.size()*input[0].size());
	for(unsigned i = 0;i < input.size();i++)
		temp.insert(temp.end(), input[i].begin(), input[i].end());
	input_mod = mat(temp);
	input_mod.reshape(input[0].size(),input.size());
	//normalization
	/*
	response_mod -= mean(response_mod);
	response_mod /= stddev(response_mod);*/
	double tempMean,tempStd;
	for(unsigned i = 0;i < input_mod.n_rows;i++){
		tempMean = mean(input_mod.row(i));
		outputMean.push_back(tempMean);
		input_mod.row(i) -= tempMean;
		tempStd = stddev(input_mod.row(i));
		outputStd.push_back(tempStd);
		input_mod.row(i) /= tempStd;
	}
	temp.clear();
	temp = conv_to< std::vector<double> >::from(response_mod);
	normal.push_back(temp);
	for(unsigned i = 0;i < input_mod.n_rows;i++){
		temp.clear();
		temp = conv_to< std::vector<double> >::from(input_mod.row(i));
		normal.push_back(temp);
	}

	return 0;

}
bool Regression::doLinearRegression(int lambda,
						vector<int >& result,
						vector<double > &cof){
							Linear_Regression lr;
							lr.useLinearRegression(input_mod,response_mod,lambda,result,cof);
							return 0;

						}

bool Regression::doLarsRegression(int lambda1,
					  			  int lambda2,
								  vector<int >& result,
					  			  vector<double >& cof){
						Lars_Regression lsr;
						lsr.useLarsRegression(input_mod,response_mod,lambda1,lambda2,result,cof);
						return 0;

					  }

