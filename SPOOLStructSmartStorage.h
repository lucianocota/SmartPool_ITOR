#ifndef SPOOLStructSmartStorage_H
#define SPOOLStructSmartStorage_H

#include "./cplexMOPoolSearch.hpp"
#include "OptFrame/MultiObjSearch.hpp"

class SPOOLStructSmartStorage: public SPOOLStruct
{
protected:
	vector<vector<double> > popGridRate;
	vector<vector<double> > popOtherRelevantValues;


public:
	SPOOLStructSmartStorage(MOMETRICS<int> _moMetrics, vector<double> _referencePointsHV, vector<double> _utopicSol);
	virtual ~SPOOLStructSmartStorage();

	vector<double> addSolToPop(const IloNumArray& vals, const IloNumVarArray& var, int nOptObj);
	void exportParetoFrontValues(string filename, int nMILPProblems, int tLim, int nOptObj, double tNow);

};

#endif /*SPOOLStructSmartStorage_H*/

