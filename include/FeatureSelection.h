#ifndef FeatureSelection_H
#define FeatureSelection_H

#include <string>
#include <vector>
#include <ostream>
#include <sstream>


using std::string;
using std::vector;
using std::ostream;
using std::ofstream;
using std::stringstream;

struct ScoreElem{
    double score;
    int id;
    ScoreElem(): score(0), id(0){}
    ScoreElem(double sc, int i): score(sc), id(i){}
    bool operator< (const ScoreElem &y) const { return this->score < y.score;}
};

class FeatureSelection{
public:
    // constructor
    FeatureSelection(string FE_fileName);
    virtual ~FeatureSelection();
    bool valid(){return valid_;}

    // data access
    vector<vector<double> > &allFeatureData(){return featureData;}
    void allSelectedData(vector<vector<double> > &storage);
    bool getAttrCol(string attrName, vector<double> &colVec);
    bool getAttrCol(unsigned attrId, vector<double> &colVec);

    // data info
    unsigned numOfFeatures(){return attrNameVec.size();}
    unsigned numOfUsedFeatures(){return useFeatureId_.size();}
    unsigned numOfTypes(){return typeNameVec.size();}
    unsigned numOfSamples(){return featureData.size();}
    int attrId(string attrName);
    int useFeatureId(int i){return useFeatureId_[i];}
    string getAttrName(int attrId){if(attrNameVec.size()>attrId) return attrNameVec[attrId]; return "";}
    string stringOut();

    // feature using and excluding
    void excludeFeature(string featureName);
    void excludeNonChangeFeature();
    void useAllFeature();
    void useFeature(string featureName);

    // discretization method
    bool disct_ew(vector<vector<double> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr=NULL);
    bool disct_ew_cycle(vector<vector<double> >& discreteData, int partitionNum);
    bool disct_col_manual(vector<double> &inFeatureData, vector<double> &discreteData, vector<double>& cutPoints, bool increaseNoBack);
    void disct_col_ew_cycle(vector<double> &inFeatureData, vector<double> &outDisctData, int partitionNum, bool increaseNoBack);

    // selection algorithm
    void JMI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void MRMR(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix,double *classColumn, vector<int> &outputId);
    void CMIM(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void DISR(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void CHI(int top_k, int noOfSamples, int noOfFeatures,double *featureMatrix, double *classColumn, vector<int> &outputId);
    void CondMI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void ICAP(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void MIM(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void FCBF(int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, double threshold, vector<int> &outputId);

    // score and rank
    void score_and_rank_mi(vector<vector<int> > &mi_rank, unsigned print_n, ostream &fout);
    void score_and_rank_regs(vector<vector<int> > &regs_rank, vector<vector<double> > &regs_coeff, double threshold, unsigned print_n, ostream &fout);

    // Euclidean Distance
    double eu_distance(vector<double>& x, vector<double>& y);

    // CSV parser
    void csvSplit(string s, const char delimiter, vector<string> &value);

private:
    bool valid_;
    stringstream sout;
    vector<vector<double> > featureData;  // contains all feature data
    vector<int> useFeatureId_; // used
    vector<double> featureDataCycle;
    vector<string> attrNameVec;
    vector<string> typeNameVec;

    double chi2f(vector<double> &feature, vector<double> &label);  // for CHI
    double SU(double *dataVector1, double *dataVector2, int vectorLength);  // for FCBF
};

#endif // FeatureSelection_H
