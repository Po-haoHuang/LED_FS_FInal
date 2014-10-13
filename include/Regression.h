#ifndef Regression_H //Linear Regression
#define Regression_H
#include <mlpack/core.hpp>
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include"../include/LinearRegression.h"
#include"../include/Lars.h"


using namespace mlpack;
using namespace mlpack::regression;
using namespace std;
using namespace arma;

class Regression{

	public:
		Regression();
		virtual ~Regression();
		
		bool init(vector<vector<double > > input,
					  vector<double > response,
					  vector<vector<double > >& normal,
					  vector<double>& outputMean,
					  vector<double>& outputStd);
		bool doLinearRegression(int lambda ,
								vector<int >& result,
								vector<double > &cof);
		bool doLarsRegression(int lambda1,
							  int lambda2,
							  vector<int >& result,
							  vector<double >& cof);
	private:
		mat input_mod;
		vec response_mod;
};


#endif





