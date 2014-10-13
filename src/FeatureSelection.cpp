#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cfloat>
#include <iomanip>

#include "../include/FeatureSelection.h"

using namespace std;

FeatureSelection::~FeatureSelection()
{
    //dtor
}

FeatureSelection::FeatureSelection(string FE_fileName)
{
    // open file
    ifstream inFile(FE_fileName.c_str(), ios::in);
    if(!inFile) {
        cout << "FS init error: Cannot open file: " << FE_fileName.c_str()<< endl;
        valid_ = false;
        return;
    }

    // read attribute title
    string lineBuffer;
    getline(inFile, lineBuffer);
    unsigned thirdCommaPos = lineBuffer.find(",");  // delete id, fid and cycle info
    thirdCommaPos = lineBuffer.find(",",thirdCommaPos+1);
    thirdCommaPos = lineBuffer.find(",",thirdCommaPos+1);
    lineBuffer = lineBuffer.substr(thirdCommaPos+1);

    // read attribute name
    csvSplit(lineBuffer, ',', attrNameVec);
    if(attrNameVec.back().size()==0)
        attrNameVec.pop_back(); // remove redundant value
    for(unsigned i=0; i<attrNameVec.size(); i++){
        string attrName = attrNameVec[i];
        string typeName = attrName.substr(0, attrName.rfind('_'));
        if(find(typeNameVec.begin(), typeNameVec.end(), typeName) == typeNameVec.end()){
            typeNameVec.push_back(typeName);
        }
    }

    // read data by line
    featureData.reserve(1024);
    vector<double> lineValue;
    vector<string> lineStrValue;
    while(getline(inFile, lineBuffer)){
        lineValue.clear();
        lineStrValue.clear();
        csvSplit(lineBuffer, ',', lineStrValue);
        featureDataCycle.push_back(atof(lineStrValue[2].c_str()));
        for(unsigned i=3; i<lineStrValue.size(); i++){
            lineValue.push_back(atof(lineStrValue[i].c_str()));
        }
        featureData.push_back(lineValue);
    }
    inFile.close();
    valid_ = true;
}


void FeatureSelection::allSelectedData(vector<vector<double> > &s)
{
    for(unsigned i=0; i<numOfSamples(); i++){
        vector<double> oneSample;
        for(unsigned j=0; j<numOfUsedFeatures(); j++){
            oneSample.push_back(featureData[i][useFeatureId(j)]);
        }
        s.push_back(oneSample);
    }
}

int FeatureSelection::attrId(string attrName)
{
    for(unsigned col=0; col<attrNameVec.size(); col++){
        if(attrNameVec[col].find(attrName) != string::npos){ // match
            return col;
        }
    }
    return -1;
}

bool FeatureSelection::getAttrCol(string attrName, vector<double> &colVec)
{
    for(unsigned col=0; col<attrNameVec.size(); col++){
        if(attrNameVec[col].find(attrName) != string::npos){ // match
            for(unsigned j=0; j<featureData.size(); j++){
                colVec.push_back(featureData[j][col]);
            }
            return true;
        }
    }
    return false;
}

bool FeatureSelection::getAttrCol(unsigned attrId, vector<double> &colVec)
{
    if(attrId < numOfFeatures()){
        for(unsigned j=0; j<numOfSamples(); j++){
            colVec.push_back(featureData[j][attrId]);
        }
        return true;
    }
    return false;
}

string FeatureSelection::stringOut()
{
    string str = sout.str();
    sout.str("");
    sout.clear();
    return str;
}

void FeatureSelection::csvSplit(string s, const char delimiter, vector<string> &value)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    while (end <= std::string::npos){
        value.push_back(s.substr(start, end-start));
        if (end == std::string::npos)
	    	break;
    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
}

void FeatureSelection::excludeFeature(string ftName)
{
    if(ftName.size()==0) return;
    for(unsigned i=0; i<useFeatureId_.size(); i++){
        if(getAttrName(useFeatureId_[i]).find(ftName)!=string::npos){  // if attribute name match
            useFeatureId_[i]=-1;  // exclude
        }
    }
    // remove unused feature id
    vector<int> newUseFeatureId;  // temporary use only
    for(unsigned i=0; i<useFeatureId_.size(); i++){
        if(useFeatureId_[i]!=-1) newUseFeatureId.push_back(useFeatureId_[i]);
    }
    useFeatureId_.swap(newUseFeatureId);
}

void FeatureSelection::excludeNonChangeFeature()
{
    for(unsigned i=0; i<useFeatureId_.size(); i++){
        vector<double> colVec;
        getAttrCol(useFeatureId_[i], colVec);  // exclude zero columns
        if(*max_element(colVec.begin(), colVec.end()) == *min_element(colVec.begin(), colVec.end())){
            useFeatureId_[i]=-1;
        }
    }
    // remove unused feature id
    vector<int> useFeatureIdReplace;  // temporary use only
    for(unsigned i=0; i<useFeatureId_.size(); i++){
        if(useFeatureId_[i]!=-1) useFeatureIdReplace.push_back(useFeatureId_[i]);
    }
    useFeatureId_.swap(useFeatureIdReplace);
}

void FeatureSelection::useAllFeature()
{
    useFeatureId_.clear();
    for(unsigned i=0; i<numOfFeatures(); i++){
        useFeatureId_.push_back(i);  // initialize: use all feature id
    }
}

void FeatureSelection::useFeature(string ftName)
{
    if(ftName.size()==0) return;
    if(ftName=="ALL"){
       useAllFeature();
       return;
    }
    for(unsigned i=0; i<numOfFeatures(); i++){
        if(attrNameVec[i].find(ftName)!=string::npos){  // if attribute name match
            bool repeated = false;
            for(unsigned j=0; j<useFeatureId_.size(); j++){
                if(attrNameVec[i]==getAttrName(useFeatureId_[j])){ // already contained
                    repeated = true;
                    break;
                }
            }
            if(!repeated) useFeatureId_.push_back(i); // add the feature
        }
    }
}

void FeatureSelection::score_and_rank_mi(vector<vector<int> > &mi_rank, unsigned print_n, ostream &fout)
{
    vector<ScoreElem> scoreVector(numOfFeatures(), ScoreElem());
    vector<ScoreElem> tScoreVector(numOfTypes(), ScoreElem());
    for(unsigned i=0; i<mi_rank.size(); i++){
        for(unsigned curRank=0; curRank<mi_rank[i].size(); curRank++){
            int id = useFeatureId(mi_rank[i][curRank]);
            scoreVector[id].id = id;
            scoreVector[id].score += numOfUsedFeatures()-curRank;  // MI score
        }
    }
    sort(scoreVector.begin(), scoreVector.end());
    reverse(scoreVector.begin(), scoreVector.end());  // to descent order

    cout << fixed << setprecision(2);
    cout << "Final score - MI:" << endl;
    fout << fixed << setprecision(2);
    fout << "Final score - MI:" << endl;
    double totalScore = 0.0;
    unsigned printCnt = 0;
    for(unsigned i=0; i<scoreVector.size(); i++){// prevent it too big.
        double relative_score = scoreVector[i].score/numOfUsedFeatures()/mi_rank.size();
        if(relative_score>0){
            if(printCnt++ < print_n){
                cout << i+1 << ": " << relative_score*100 << "% ";
                cout << getAttrName(scoreVector[i].id) << endl;
                fout << i+1 << "," << relative_score*100 << "%,";
                fout << getAttrName(scoreVector[i].id) << endl;
            }
            // counting the portion of each type
            for(unsigned ti=0; ti<typeNameVec.size(); ti++){
                if(getAttrName(scoreVector[i].id).find(typeNameVec[ti])!=string::npos){
                    tScoreVector[ti].id = ti;
                    tScoreVector[ti].score += relative_score;
                    totalScore += relative_score;
                }
            }
        }
    }
    cout << endl << "Type Analysis - MI:" << endl;
    fout << endl << "Type Analysis - MI:" << endl;
    printCnt = 0;
    sort(tScoreVector.begin(), tScoreVector.end());
    reverse(tScoreVector.begin(), tScoreVector.end());  // to descent order
    for(unsigned i=0; i<tScoreVector.size(); i++){
        double tScore = tScoreVector[i].score/totalScore;
        if(printCnt++ < print_n  && tScore>0){
            cout << i+1 << ": " << tScore*100 << "% ";
            cout << typeNameVec[tScoreVector[i].id] << endl;
            fout << i+1 << "," << tScore*100 << "%,";
            fout << typeNameVec[tScoreVector[i].id] << endl;
        }
    }

    cout << endl;
    fout << endl;
}

bool elemZero(ScoreElem x){
    return x.score==0;
}

void FeatureSelection::score_and_rank_regs(vector<vector<int> > &regs_rank, vector<vector<double> > &regs_coeff, double threshold, unsigned print_n, ostream &fout)
{
    vector<ScoreElem> scoreVector(numOfFeatures(), ScoreElem());
    vector<ScoreElem> tScoreVector(numOfTypes(), ScoreElem());
    for(unsigned i=0; i<regs_rank.size(); i++){
        for(unsigned curRank=0; curRank<regs_rank[i].size(); curRank++){
            int id = useFeatureId(regs_rank[i][curRank]);
            scoreVector[id].id = id;
            // Regs score
            scoreVector[id].score += regs_coeff[i][curRank];
        }
    }
    sort(scoreVector.begin(), scoreVector.end());
    reverse(scoreVector.begin(), scoreVector.end());  // convert to descent order

    cout << fixed << setprecision(2);
    cout << "Final score - Regression:" << endl;
    fout << fixed << setprecision(2);
    fout << "Final score - Regression:" << endl;
    double totalScore = 0.0;
    int printCnt = 0;
    for(unsigned i=0; i<scoreVector.size(); i++){
        double relative_score = scoreVector[i].score / scoreVector[0].score;
        if(relative_score>0){
            if(printCnt++ < print_n){
                cout << i+1 << ": " << relative_score*100 << "% ";
                cout << getAttrName(scoreVector[i].id) << endl;
                fout << i+1 << "," << relative_score*100 << "%,";
                fout << getAttrName(scoreVector[i].id) << endl;
            }
            // counting the portion of each type
            for(unsigned ti=0; ti<typeNameVec.size(); ti++){
                if(getAttrName(scoreVector[i].id).find(typeNameVec[ti])!=string::npos){
                    tScoreVector[ti].id = ti;
                    tScoreVector[ti].score += relative_score;
                    totalScore += relative_score;
                }
            }
        }
    }
    cout << "0: -----" << endl;
    fout << "0,-----" << endl;

    // print top n negative feature
    vector<ScoreElem>::reverse_iterator ritEnd = find_if(scoreVector.rbegin(), scoreVector.rend(), elemZero);
    printCnt = 0;
    for(vector<ScoreElem>::reverse_iterator it=scoreVector.rbegin(); it!=ritEnd; ++it){
        double relative_score = -it->score / scoreVector.rbegin()->score;
        if(printCnt++ < print_n){
            cout << -printCnt << ": " << relative_score*100 << "% ";
            cout << getAttrName(it->id) << endl;
            fout << -printCnt << "," << relative_score*100 << "%,";
            fout << getAttrName(it->id) << endl;
        }else
            break;
    }

    cout << endl << "Type Analysis - Regression:" << endl;
    fout << endl << "Type Analysis - Regression:" << endl;
    printCnt = 0;
    sort(tScoreVector.begin(), tScoreVector.end());
    reverse(tScoreVector.begin(), tScoreVector.end());  // to descent order
    for(unsigned i=0; i<tScoreVector.size(); i++){
        double tScore = tScoreVector[i].score/totalScore;
        if(printCnt++ < print_n && tScore>0){
            cout << i+1 << ": " << tScore*100 << "% ";
            cout << typeNameVec[tScoreVector[i].id] << endl;
            fout << i+1 << "," << tScore*100 << "%,";
            fout << typeNameVec[tScoreVector[i].id] << endl;
        }
    }

    cout << endl;
    fout << endl;
}
