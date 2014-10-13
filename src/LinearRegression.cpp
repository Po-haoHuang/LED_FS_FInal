#include"..\include\LinearRegression.h"

Linear_Regression::Linear_Regression(){
	//c_tor
}
Linear_Regression::~Linear_Regression(){
	//d_tor
}

bool Linear_Regression::useLinearRegression(arma::mat& data,
								  			arma::vec& responses,
								  			double lambda,
  						  					vector<int>& result,
											vector<double>& cof){

	//check if x , y matches
	if (responses.n_elem != data.n_cols)
		Log::Fatal << "Number of responses must be equal to number of rows of X!"
		<< endl;
	//usage of linear regression(Lambda = 0)
	//and ridge regression(set lambda = 1 or 2 or 3)
	//Construct LR model
	lr = LinearRegression(data,responses,lambda);
	//Obtain beta coefficient
	vec tempPara = lr.Parameters();
	//remove the B0 of linear regression(constant beta 0)
	tempPara.shed_row(0);
	try{
		cof = conv_to< std::vector<double> >::from(sort(tempPara,"d"));
	}catch(exception& e){
		cerr << "exception caught: " << e.what() << endl;
        return 0;
	}
	//sort the beta coefficient
	uvec temp;
	try{
		temp = sort_index(tempPara,"d");
	}catch(exception& e){
		cerr << "exception caught: " << e.what() << endl;
        return 0;
	}

	for(unsigned i = 0;i < data.n_rows ;i++)
		result.push_back(temp[i]);
	return 0;
}
