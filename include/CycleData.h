#ifndef CYCLEDATA_H
#define CYCLEDATA_H

#include <vector>
#include "FileData.h"

using std::vector;

class CycleData{
public:
    CycleData();
    virtual ~CycleData();
    double cycle;
    int nCycle;
    bool valid;
    vector<FileData> fileDataVector;

    unsigned cycleSize(){ return fileDataVector.size();}

private:
};

#endif // CYCLEDATA_H
