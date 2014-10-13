#ifndef Linear_Regression_H //Linear Regression
#define Linear_Regression_H

//include from mlpack
#include <mlpack/core.hpp>
#include "mlpack/methods/linear_regression/linear_regression.hpp"


using namespace mlpack;
using namespace mlpack::regression;
using namespace std;
using namespace arma;

class Linear_Regression{
	
public:
	Linear_Regression();
	virtual ~Linear_Regression();
	bool useLinearRegression(arma::mat& data,
							 arma::vec& responses,
							 double lambda,
							 vector<int>& result,
							 vector<double>& cof);
private:
	// Regress.
	LinearRegression lr;
	
};
#endif// LR
