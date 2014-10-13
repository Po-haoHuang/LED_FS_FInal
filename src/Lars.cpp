#include"../include/Lars.h"

Lars_Regression::Lars_Regression(){
	//c_tor
}
Lars_Regression::~Lars_Regression(){
	//d_tor
}
bool Lars_Regression::useLarsRegression(arma::mat& data,
					   arma::vec& responses,
					   double lambda1,
					   double lambda2,
					   vector<int>& result,
					   vector<double>& cof){
	vec beta;
	// Make sure x , y matches
	if (responses.n_elem != data.n_cols)
		Log::Fatal << "Number of responses must be equal to number of rows of X!"
			<< endl;
	//initialize LARS regression
	/*usage of LARS: Lasso regression(lambda1 = 1 or 2 or 3 lambda2 =0)
	and elastic net(lambda1 = 1 or 2 or 3 lambda2 =1 or 2 or 3)*/
	LARS lars(false, lambda1, lambda2);
	//Construct LARS model
	try{
        lars.Regress(data, responses, beta, true );
	}catch(exception& e){
        cerr << "exception caught: " << e.what() << endl;
        return 1;
	}
	try{
	cof = conv_to< std::vector<double> >::from(sort(beta,"d"));
	}catch(exception& e){
		cerr << "exception caught: " << e.what() << endl;
        return 0;
	}
	//sort the beta coefficient
	uvec temp;
	try{
		temp = sort_index(beta,"d");
	}catch(exception& e){
		cerr << "exception caught: " << e.what() << endl;
        return 0;
	}
	for(unsigned i = 0;i < data.n_rows ;i++)
		result.push_back(temp[i]);

	return 0;
}
