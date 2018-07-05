#ifndef READPARETOSETS_HPP_
#define READPARETOSETS_HPP_

#include "OptFrame/MultiObjSearch.hpp"
#include "OptFrame/RandGen.hpp"

#include <unistd.h>

using namespace std;
using namespace optframe;

class readParetoSets
{
public:
	MOMETRICS<int> moMetrics;
	int nOptObj;
public:

	readParetoSets(int _nOptObj, MOMETRICS<int> _moMetrics) :
			nOptObj(_nOptObj), moMetrics(_moMetrics)
	{

	}

	~readParetoSets()
	{

	}

	vector<vector<double> > readParetoFronts(vector<string>& vInputModel, vector<vector<vector<double> > >& vParetoSet)
	{
		vector<vector<double> > paretoSetRef;
		for (int iM = 0; iM < vInputModel.size(); iM++)
		{
			cout << "Reading file: " << vInputModel[iM] << "....." << endl;

			Scanner scanner(new File(vInputModel[iM]));
			vector<vector<double> > paretoSet;
			while (scanner.hasNextDouble())
			{
				vector<double> fo;
				bool add = true;
				for (int o = 0; o < nOptObj; o++)
				{
					if (scanner.hasNextDouble())
					{
						fo.push_back(scanner.nextDouble());
					}
					else
					{
						add = false;
						break;
					}
				}
				if (add)
					paretoSet.push_back(fo);

			}
			moMetrics.unionSets(paretoSetRef, paretoSet);
			vParetoSet.push_back(paretoSet);
		}

		return paretoSetRef;
	}

	vector<vector<double> > readParetoFrontsFrom1ToObj(vector<string>& vInputModel, vector<vector<vector<double> > >& vParetoSet)
	{
		vector<vector<double> > paretoSetRef;
		for (int iM = 0; iM < vInputModel.size(); iM++)
		{
			cout << "Reading file: " << vInputModel[iM] << "....." << endl;

			Scanner scanner(new File(vInputModel[iM]));
			vector<vector<double> > paretoSet;
			while (scanner.hasNextDouble())
			{
				string nextLine = scanner.nextLine();
				Scanner scannerLine(nextLine.c_str());
				vector<double> fo;
				bool add = true;
				for (int o = 0; o < nOptObj; o++)
				{
					if (scannerLine.hasNextDouble())
					{
						fo.push_back(scannerLine.nextDouble());
					}
					else
					{
						add = false;
						break;
					}
				}
				if (add)
					paretoSet.push_back(fo);

			}
			moMetrics.unionSets(paretoSetRef, paretoSet);
			vParetoSet.push_back(paretoSet);
		}

		return paretoSetRef;
	}

	void exec(vector<string> vInputModel, vector<double> utopicSol, vector<double> referencePointsHV, string paretoOutput)
	{
		cout << "Exec Math read Pareto Sets ! \n Be sure to insert an UtopicSol and References points" << endl;

		vector<vector<vector<double> > > vParetoSet;

		vector<vector<double> > paretoSetRef = readParetoFrontsFrom1ToObj(vInputModel, vParetoSet);

		vParetoSet.push_back(paretoSetRef);

		vector<vector<double> > vIndicadoresQualidade;
		cout << "===================================" << endl;

		for (int nPS = 0; nPS < vParetoSet.size(); nPS++)
		{
			vector<vector<double> > paretoSet = vParetoSet[nPS];
			vector<double> indicadores;
			indicadores.push_back(paretoSet.size());
			//printVectorPareto(paretoSetRef);
			//cout << "conjuntoParetoAtual:" << endl;
			//printVectorPareto(paretoSet);
			double cardinalidade = moMetrics.cardinalite(paretoSet, paretoSetRef);
			double setCover = moMetrics.setCoverage(paretoSet, paretoSetRef);
			double spacing = moMetrics.spacing(paretoSet);
			indicadores.push_back(cardinalidade);
			indicadores.push_back(setCover);
			indicadores.push_back(spacing);

			double hv = -1;
			double delta = -1;
			if (utopicSol.size() > 0)
				delta = moMetrics.deltaMetric(paretoSet, utopicSol, true);

			if ((referencePointsHV.size() > 0) && (nPS < (vParetoSet.size() - 1)))
				hv = moMetrics.hipervolumeWithExecRequested(paretoSet, referencePointsHV, true);

			indicadores.push_back(delta);
			indicadores.push_back(hv);
			vIndicadoresQualidade.push_back(indicadores);

			if (nPS < (vParetoSet.size() - 1))
				cout << "instance: " << vInputModel[nPS] << endl;
			else
				cout << "instance: Pareto Reference" << endl;

			cout << "size: " << paretoSet.size() << "/" << paretoSetRef.size() << endl;
			cout << "card: " << cardinalidade << endl;
			cout << "CS: " << setCover << endl;
			cout << "spacing: " << spacing << endl;
			cout << "delta: " << delta << endl;
			cout << "hv: " << hv << "\n" << endl;
		}

		cout << "printing pareto REF" << endl;
		cout << paretoSetRef << endl;
		cout << "printing Indicadores" << endl;
		cout.precision(10);
		cout << vIndicadoresQualidade << endl;

		cout << "writing Pareto at file:" << paretoOutput << endl;
		FILE* fFronteiraPareto = fopen(paretoOutput.c_str(), "w");
		for (int nS = 0; nS < paretoSetRef.size(); nS++)
		{
			for (int nE = 0; nE < nOptObj; nE++)
			{
				fprintf(fFronteiraPareto, "%.5f\t", paretoSetRef[nS][nE]);
			}
			fprintf(fFronteiraPareto, "\n");
		}

		fclose(fFronteiraPareto);
		cout << "File wrote with success!" << endl;

		getchar();

	}

	vector<double> getParetoFrontIndicators(string instanceName, vector<vector<double> > paretoSet, vector<vector<double> > paretoSetRef, vector<double> utopicSol, vector<double> referencePointsHV)
	{
		vector<double> metricValues;
		double cardinalidade = moMetrics.cardinalite(paretoSet, paretoSetRef);
		double setCover = moMetrics.setCoverage(paretoSet, paretoSetRef);
		double spacing = moMetrics.spacing(paretoSet);

		double hv = -1;
		double delta = -1;
		if (utopicSol.size() > 0)
			delta = moMetrics.deltaMetric(paretoSet, utopicSol, true);

		if (referencePointsHV.size() > 0)
			hv = moMetrics.hipervolumeWithExecRequested(paretoSet, referencePointsHV, true);

		metricValues.push_back(paretoSet.size());
		metricValues.push_back(cardinalidade);
		metricValues.push_back(setCover);
		metricValues.push_back(spacing);
		metricValues.push_back(delta);
		metricValues.push_back(hv);

		cout << "instance: " << instanceName << endl;

		cout << "size: " << metricValues[0] << "/" << paretoSetRef.size() << endl;
		cout << "card: " << metricValues[1] << endl;
		cout << "CS: " << metricValues[2] << endl;
		cout << "spacing: " << metricValues[3] << endl;
		cout << "delta: " << metricValues[4] << endl;
		cout << "hv: " << metricValues[5] << "\n" << endl;
		return metricValues;
	}
	void execForBatches(vector<string> vInputModel, vector<double> utopicSol, vector<double> referencePointsHV, string paretoOutput, int nBatches, int nDiffConfigurations)
	{
		cout << "Exec Math read Pareto Sets ! \n Be sure to insert an UtopicSol and References points" << endl;

		vector<vector<vector<double> > > vParetoSet;

		vector<vector<double> > paretoSetRef = readParetoFrontsFrom1ToObj(vInputModel, vParetoSet);

		vParetoSet.push_back(paretoSetRef);

		int nIndicators = 6;
		vector<vector<double> > vIndicadoresQualidade(nDiffConfigurations);
		for (int dC = 0; dC < nDiffConfigurations; dC++)
			for (int m = 0; m < nIndicators; m++)
				vIndicadoresQualidade[dC].push_back(0);

		cout << "===================================" << endl;
		for (int nB = 0; nB < nBatches; nB++)
			for (int dC = 0; dC < nDiffConfigurations; dC++)
			{
				int currentPareto = dC + nB * nDiffConfigurations;

				vector<vector<double> > paretoSet = vParetoSet[currentPareto];

				vector<double> indicadores = getParetoFrontIndicators(vInputModel[currentPareto], paretoSet, paretoSetRef, utopicSol, referencePointsHV);

				if (indicadores.size() != nIndicators)
				{
					cout << "errors on number of indicators" << endl;
					exit(1);
				}

				for (int m = 0; m < nIndicators; m++)
					vIndicadoresQualidade[dC][m] += indicadores[m];

			}
		for (int dC = 0; dC < nDiffConfigurations; dC++)
			for (int m = 0; m < nIndicators; m++)
				vIndicadoresQualidade[dC][m] /= nBatches;

		cout << "===================================" << endl;

		cout << "Printing Pareto Reference Set...." << endl;
		vector<double> indicadores = getParetoFrontIndicators("PARETO SET", paretoSetRef, paretoSetRef, utopicSol, referencePointsHV);
		cout << "===================================" << endl;
		cout << paretoSetRef << endl;
		cout << "Printing average values for each configuration" << endl;
		cout.precision(10);
		cout << vIndicadoresQualidade << endl;

		stringstream ss;
		ss << paretoOutput << "_Pareto";
		cout << "Writing Pareto Reference at file " << ss.str().c_str() << endl;

		FILE* fFronteiraPareto = fopen(ss.str().c_str(), "w");
		for (int nS = 0; nS < paretoSetRef.size(); nS++)
		{
			for (int nE = 0; nE < nOptObj; nE++)
			{
				fprintf(fFronteiraPareto, "%.5f\t", paretoSetRef[nS][nE]);
			}
			fprintf(fFronteiraPareto, "\n");
		}
		fclose(fFronteiraPareto);

		ss.clear();
		ss.str("");
		ss << paretoOutput << "_AVGResults";
		cout << "Writing average values for each configuration at " << ss.str().c_str() << endl;

		FILE* fAVGResults = fopen(ss.str().c_str(), "w");
		fprintf(fAVGResults, "PFsize\tcard\tSC\tSpacing\tDelta\tHV\n");
		for (int dc = 0; dc < vIndicadoresQualidade.size(); dc++)
		{
			for (int m = 0; m < nIndicators; m++)
			{
				fprintf(fAVGResults, "%.5f\t", vIndicadoresQualidade[dc][m]);
			}
			fprintf(fAVGResults, "\n");
		}
		fclose(fAVGResults);

		cout << "File wrote with success!" << endl;

		getchar();

	}

// 2

};

#endif /* READPARETOSETS_HPP_ */
