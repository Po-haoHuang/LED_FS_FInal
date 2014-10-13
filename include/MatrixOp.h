#ifndef MatrixOp_H
#define MatrixOp_H

#include <vector>

using std::vector;

// element-wise matrix operation
void elemMul(vector<double> &result, vector<double> &v1, vector<double> &v2)
{
    result = vector<double>(v1.size(),0);
    for(unsigned i=0; i<v1.size(); i++) {
        result[i] = v1[i]*v2[i];
    }
}

void elemAdd(vector<double> &result, vector<double> &v1, vector<double> &v2)
{
    result = vector<double>(v1.size(),0);
    for(unsigned i=0; i<v1.size(); i++) {
        result[i] = v1[i]+v2[i];
    }
}

void elemSub(vector<double> &result, vector<double> &v1, vector<double> &v2)
{
    result = vector<double>(v1.size(),0);
    for(unsigned i=0; i<v1.size(); i++) {
        result[i] = v1[i]-v2[i];
    }
}

void elemDiv(vector<double> &result, vector<double> &v1, vector<double> &v2)
{
    result = vector<double>(v1.size(),0);
    for(unsigned i=0; i<v1.size(); i++) {
        result[i] = v1[i]/v2[i];
    }
}

void elemDividedByConst(vector<double> &result, vector<double> &v, double c)
{
    result = vector<double>(v.size(),0);
    for(unsigned i=0; i<v.size(); i++) {
        result[i] = v[i]/c;
    }
}

void elemSquare(vector<double> &result, vector<double> &v)
{
    result = vector<double>(v.size(),0);
    for(unsigned i=0; i<v.size(); i++) {
        result[i] = v[i]*v[i];
    }
}

void matrixColSum(vector<double> &result, vector<double> &matrix, unsigned numOfCol)
{
    result = vector<double>(numOfCol,0);
    for(unsigned i=0; i<numOfCol; i++) {
        for(unsigned j=0; j<matrix.size()/numOfCol; j++) {
            result[i] += matrix[i+j*numOfCol];
        }
    }
}

void matrixRowSum(vector<double> &result, vector<double> &matrix, unsigned numOfRow)
{
    result = vector<double>(numOfRow,0);
    for(unsigned i=0; i<numOfRow; i++) {
        for(unsigned j=0; j<matrix.size()/numOfRow; j++) {
            result[i] += matrix[i*matrix.size()/numOfRow + j];
        }
    }
}

void matrixRowRepeat(vector<double> &v, unsigned times)
{
    vector<double> origVec;
    for(unsigned i=0; i<v.size(); i++) {
        origVec.push_back(v[i]);
    }
    v = vector<double>(origVec.size()*times);
    for(unsigned i=0; i<v.size(); i++) {
        v[i] = origVec[i%origVec.size()];
    }
}

void matrixColRepeat(vector<double> &v, unsigned times)
{
    vector<double> origVec;
    for(unsigned i=0; i<v.size(); i++) {
        origVec.push_back(v[i]);
    }
    v = vector<double>(origVec.size()*times);
    for(unsigned i=0; i<v.size(); i++) {
        v[i] = origVec[i/times];
    }
}

#endif
