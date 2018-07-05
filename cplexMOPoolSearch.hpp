/*
 * OptimalLinearRegression.hpp
 *
 *  Created on: 26/10/2014
 *      Author: vitor
 */

#ifndef CPLEXMOPOOLSEARCH_HPP_
#define CPLEXMOPOOLSEARCH_HPP_

#include <ilcplex/ilocplex.h>
#include <ilcplex/cplexx.h>
#include <ilcplex/ilocplexi.h>

#include <iterator>
#include <algorithm>
#include <numeric>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>      // std::ifstream
#include <sstream>
#include "OptFrame/RandGen.hpp"
#include "OptFrame/Timer.hpp"
#include "OptFrame/Util/printable.h"
#include "OptFrame/MultiObjSearch.hpp"

#include <vector>

ILOSTLBEGIN

using namespace std;
using namespace optframe;

struct MIPStartSolution
{
	vector<double> objValues;
	string filename;
	MIPStartSolution(vector<double> _obj, string _filename) :
			objValues(_obj), filename(_filename)
	{
	}
};

class SPOOLStruct
{
public:
	MOMETRICS<int> moMetrics;
	vector<vector<double> > popObjValues;
	vector<vector<double> > paretoSET;

	vector<double> referencePointsHV;
	vector<double> utopicSol;

	SPOOLStruct(MOMETRICS<int> _moMetrics, vector<double> _referencePointsHV, vector<double> _utopicSol) :
			moMetrics(_moMetrics), referencePointsHV(_referencePointsHV), utopicSol(_utopicSol)
	{

	}

	virtual ~SPOOLStruct()
	{

	}

	vector<vector<double> > getPopObjValues()
	{
		return popObjValues;
	}

	void updatedParetoSet()
	{
		paretoSET.clear();
		paretoSET = moMetrics.createParetoSet(popObjValues);
	}

	vector<vector<double> > getParetoSet()
	{
		updatedParetoSet();
		return paretoSET;
	}

	virtual vector<double> addSolToPop(const IloNumArray& vals, const IloNumVarArray& var, int nOptObj)
	{
		vector<double> solObj;
		for (int o = 0; o < nOptObj; o++)
		{
			solObj.push_back(vals[o]);
			cout << "obj(" << o + 1 << "): " << solObj[o] << "\t";
		}
//						cout << "obj(" << 7 << "): " << finalOF[7] << "\t"; //print excess auxiliary variable
		cout << "\n";
		popObjValues.push_back(solObj);
		return solObj;
	}

	int getPopSize()
	{
		return popObjValues.size();
	}

	double getParetoHyperVolume()
	{
		return moMetrics.hipervolumeWithExecRequested(paretoSET, referencePointsHV, true);
	}

	double getParetoDeltaMetric()
	{
		return moMetrics.deltaMetric(paretoSET, referencePointsHV, true);
	}

	virtual void exportParetoFrontValues(string filename, int nMILPProblems, int tLim, int nOptObj, double tNow){

		int nParetoInd = paretoSET.size();
		stringstream ss;
		ss << "./ResultadosFronteiras/" << filename << "NExec" << nMILPProblems << "TLim" << tLim; // << "-bestMIPStart";
		cout << "Writing PF at file: " << ss.str() << "..." << endl;
		FILE* fFronteiraPareto = fopen(ss.str().c_str(), "w");
		for (int nS = 0; nS < nParetoInd; nS++)
		{
			for (int nE = 0; nE < nOptObj; nE++)
			{
				fprintf(fFronteiraPareto, "%.5f\t", paretoSET[nS][nE]);
			}
			fprintf(fFronteiraPareto, "\n");
		}
		fprintf(fFronteiraPareto, "%.5f \n", tNow);
		fprintf(fFronteiraPareto, "hv: %.5f \n", getParetoHyperVolume());
		fprintf(fFronteiraPareto, "delta: %.5f \n", getParetoDeltaMetric());
		fclose(fFronteiraPareto);
		cout << "File wrote with success!" << endl;
	}
};

class cplexMOPoolSearch
{

public:
	RandGen& rg;
	SPOOLStruct& spoolStruct;

	cplexMOPoolSearch(RandGen& _rg, SPOOLStruct& _spoolStruct) :
			rg(_rg), spoolStruct(_spoolStruct)

	{

	}

	~cplexMOPoolSearch()
	{

	}

	vector<vector<double> > exec(string filename, bool mipStart, vector<vector<double> > vMILPCoefs, int tLim, int nOptObj, int nCriteria, int maxTriesWithTLimUntilFirstFeasible)
	{
		cout << "===================================== " << endl;
		cout << "Exec CPLEX MO Pool Search Matheuristic!" << endl;

		cout << "Number of objectives functions: " << nOptObj << endl;

		vector<vector<double> > paretoSET;

		int nMILPProblems = vMILPCoefs.size();
		cout << "nMILPProblems = " << nMILPProblems << endl;

		IloEnv env;
		try
		{
			Timer tTotal;
			IloCplex cplex(env);
			IloModel model(env);
			IloObjective obj;
			IloNumVarArray var(env);
			IloRangeArray rng(env);

			string modelLPAdress = "./LP/" + filename + ".lp";
			cout << "solving: " << modelLPAdress << endl;

			cplex.importModel(model, modelLPAdress.c_str(), obj, var, rng);
			cplex.setParam(cplex.TiLim, tLim);

			vector<MIPStartSolution> poolMIPStart;

			//=========================================================================================
			// Calling MILP problems

			for (int milpProblems = 0; milpProblems < nMILPProblems; milpProblems++)
			{
				cout << "=====================================\n";
				cout << "Creating MILP model " << milpProblems << "/" << nMILPProblems << " with: \n";
				for (int o = 0; o < nOptObj; o++)
					cout << "lambda(" << o + 1 << "): " << vMILPCoefs[milpProblems][o] << "\t";
				cout << "\n";

				for (int o = 0; o < nOptObj; o++)
					obj.setLinearCoef(var[o], vMILPCoefs[milpProblems][o]);

				cout << obj << endl;
//							getchar();

				cplex.extract(model);

				//Option that read MST of the the previous solves MILP problem -- TODO
				//Maybe a list of MST can be passed as optional vector of the exec, with path directing to previous MST
				//Which may be repaired
			   //cplex.readMIPStarts("tempMIPStart/mst0.mst");

				int totalNMIPStartSolutions = poolMIPStart.size();
				// =====================================================
				//Read the file with the best MIP start // TODO -- for minimization problems - otherwise, it will get the worse one
				if (totalNMIPStartSolutions > 0)
				{
					cout << "Pool of MIP start solution has: " << totalNMIPStartSolutions << " possibilities" << endl;

					double bestFO;
					int bestMIPStartIndex; //
					//Selecting best MIP start for the current weights
					for (int mipS = 0; mipS < totalNMIPStartSolutions; mipS++)
					{
						double currentMILPObj = 0;
						for (int o = 0; o < nOptObj; o++)
							currentMILPObj += vMILPCoefs[milpProblems][o] * poolMIPStart[mipS].objValues[o];

						if (mipS == 0)
						{
							bestFO = currentMILPObj; //Initialize bestFO with the first value
							bestMIPStartIndex = mipS;
						}

						if (currentMILPObj < bestFO)
						{
							bestFO = currentMILPObj;
							bestMIPStartIndex = mipS;
						}
					}
					cout << "best MIPStart solution is: " << bestFO << "\t index:" << bestMIPStartIndex << endl << endl;
					cplex.readMIPStart(poolMIPStart[bestMIPStartIndex].filename.c_str());
				}
				// =====================================================

				//======================================================
				//Calls Cplex Optimization
				int nCplexPoolOfSolutions = 0;
				IloBool solveBool;
				if (milpProblems == 0)
				{
					//In the first optimization, tries several times until find first feasible
					solveBool = cplex.solve();
					nCplexPoolOfSolutions = cplex.getSolnPoolNsolns();
					int nTries = 1;
					while ((nTries < maxTriesWithTLimUntilFirstFeasible) && nCplexPoolOfSolutions == 0)
					{
						cout << "\n Any feasible solution was in " << nTries << "/" << maxTriesWithTLimUntilFirstFeasible << " tries! \n \n";
						solveBool = cplex.solve();
						nCplexPoolOfSolutions = cplex.getSolnPoolNsolns();
						nTries++;
					}
				}
				else
				{
					solveBool = cplex.solve();
					nCplexPoolOfSolutions = cplex.getSolnPoolNsolns();
				}
				//======================================================

				if (!solveBool)
				{
//								env.error() << "Failed to optimize LP" << endl;
//								throw(-1);
					cout << "=====================================\n";
					cout << "Any solution was in the current iteration! The following parameters were used: \n";
					for (int o = 0; o < nOptObj; o++)
						cout << "lambda(" << o + 1 << "): " << vMILPCoefs[milpProblems][o] << "\t";
					cout << "\n";
					cout << "\t cplex.TiLim: " << tLim << "\n";
					cout << "=====================================\n";
				}
				else
				{
					cout << "=====================================\n";
					cout << "Solved with sucess! \n";
					cout << nCplexPoolOfSolutions << " solutions were obtained! \n";
					cout << "=====================================\n\n";
				}

				IloNumArray vals(env);

				if (nCplexPoolOfSolutions > 0)
					for (int nS = 0; nS < nCplexPoolOfSolutions; nS++)
					{
						cout << "=====================================\n";
						cout << "Extracting solution: " << nS + 1 << "/" << nCplexPoolOfSolutions << endl;

						//Extracting values from each solution
						cplex.getValues(vals, var, nS);

						vector<double> solObj = spoolStruct.addSolToPop(vals, var, nOptObj);

						//==============================================
						// Write MST solutions in different files
						if (mipStart)
						{
							cout << "Writing MST solution...." << endl;
							stringstream mstFilename;
							mstFilename << "./tempMIPStart/mst" << poolMIPStart.size() << ".mst";
//							cplex.writeMIPStarts(mstFilename.str().c_str(), nS, nCplexPoolOfSolutions);
							cplex.writeMIPStart(mstFilename.str().c_str(), nS);
							MIPStartSolution mipStartSol(solObj, mstFilename.str());
							poolMIPStart.push_back(mipStartSol);
							cout << "MST Saved in file with sucess!" << endl;
							//(WriteLevel, CPX_PARAM_WRITELEVEL) //0 to 4
							//CPX_WRITELEVEL_ALLVARS, CPX_WRITELEVEL_DISCRETEVARS, CPX_WRITELEVEL_NONZEROVARS,CPX_WRITELEVEL_NONZERODISCRETEVARS
						}
						//==============================================
						cout << "Solution: " << nS + 1 << "/" << nCplexPoolOfSolutions << " has been extracted with success and added to the population" << endl;
						cout << "=====================================\n";

					}

				cout << "=====================================\n\n";

			}

			// All problem were called!
			//=========================================================================================

			int nObtainedSolutions = spoolStruct.getPopSize();
			cout << "Total time spent: " << tTotal.now() << endl;
			cout << "Size of the obtained population:" << nObtainedSolutions << endl;

			if (nObtainedSolutions > 0)
			{
				cout << "Printing obtained population of solutions with size: " << nObtainedSolutions << endl;
				cout << spoolStruct.getPopObjValues() << endl;

				paretoSET = spoolStruct.getParetoSet();
				double nParetoInd = paretoSET.size();

				cout << "Printing Pareto Front of size: " << paretoSET.size() << endl;
				cout << paretoSET << endl;

				spoolStruct.exportParetoFrontValues(filename,nMILPProblems,tLim,nOptObj,tTotal.now());
			}
			else
			{
				cout << "Any solution was obtained among the: " << nMILPProblems << "  MILP optimizations with different weighted-sum!" << endl;
			}

			//cout << vals[vals.getSize()] << endl;
			try
			{ // basis may not exist
				IloCplex::BasisStatusArray cstat(env);
				cplex.getBasisStatuses(cstat, var);
				env.out() << "Basis statuses  = " << cstat << "\n";
			} catch (...)
			{
			}

			//env.out() << "Maximum bound violation = " << cplex.getQuality(IloCplex::MaxPrimalInfeas) << endl;
		} catch (IloException& e)
		{
			cerr << "Concert exception caught: " << e << endl;
		} catch (...)
		{
			cerr << "Unknown exception caught" << endl;
		}

		env.end();
		cout << "MO Pool Search finished com sucesso!" << endl;
		cout << "===================================== \n" << endl;
		return paretoSET;
	}

};

#endif /* CPLEXMOPOOLSEARCH_HPP_ */
