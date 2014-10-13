#ifndef Lars_Regression_H //Linear Regression
#define Lars_Regression_H
#include <mlpack/core.hpp>
#include "mlpack/methods/lars/lars.hpp"


using namespace mlpack;
using namespace mlpack::regression;
using namespace std;
using namespace arma;

class Lars_Regression{

public:
	Lars_Regression();
	virtual ~Lars_Regression();
	bool useLarsRegression(arma::mat& data,
							 arma::vec& responses,
							 double lambda1,
							 double lambda2,
							 vector<int>& result,
							 vector<double>& cof);
private:
};
#endif// LR
