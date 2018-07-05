#include "SPOOLStructSmartStorage.h"

SPOOLStructSmartStorage::SPOOLStructSmartStorage(MOMETRICS<int> _moMetrics, vector<double> _referencePointsHV, vector<double> _utopicSol) :
		SPOOLStruct(_moMetrics, _referencePointsHV, _utopicSol)
{

}

SPOOLStructSmartStorage::~SPOOLStructSmartStorage()
{

}

vector<double> SPOOLStructSmartStorage::addSolToPop(const IloNumArray& vals, const IloNumVarArray& var, int nOptObj)
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

	vector<vector<vector<bool> > > energyCharging;
	vector<vector<vector<bool> > > energyDischarging;
	vector<vector<double> > batteryRate;
	vector<double> gridRate;
	//
	int nDiscrete = 100;
	int fH = 120;
	//int nPEVs = 3;
	int nVals = vals.getSize();
	//Algebra for reaching number of PEVs
//	nVals = (fH * ((2 * D * P) + 2) ) + nOptObj + P*fH * 2fH
//	nVals - 6 - 2fH = P.fH + 2.D.P.fH + 2fH
//	nVals - 6 - 4fH = P (fH + 2.D.fH)
//	P = nVals - 6 - 4fH / (fH + 2.D.fH)

	int nPEVs = (nVals - 6 - 4 * fH) / (fH + 2 * fH * nDiscrete);
	int nVariablesPerHorizon = ((2 * nDiscrete * nPEVs) + 2);
	int nTotal = (fH * nVariablesPerHorizon) + nOptObj;


	cout << "nVals:" << vals.getSize() << endl;
	cout << "nPEVs:" << nPEVs << endl;
	cout << "nVariablesPerInterval:" << nVariablesPerHorizon << endl;
	cout << "nTotal:" << nTotal << endl;
	cout << "nVals:" << nVals << endl;

	for (int nV = nOptObj; nV < nTotal; nV += nVariablesPerHorizon)
	{
		double energySold = vals[nV];
		double energyBought = vals[nV + 1];

		if (energyBought > energySold)
			gridRate.push_back(energyBought);
		else
			gridRate.push_back(energySold * -1);

//		cout << "energyBought = " << energyBought << endl;
//		cout << "energySold = " << energySold << endl;
//		cout << var[nV] << endl;
//		cout << var[nV + 1] << endl;
//		cout<<nV<<endl;
//		getchar();

		if ((energySold > 0.00001) && (energyBought > 0.00001))
		{
			cout << "BUG!" << endl;
			cout << "energyBought = " << energyBought << endl;
			cout << "energySold = " << energySold << endl;
			//cout << var << endl;
			//cout << vals << endl;
			getchar();
		}

		vector<vector<bool> > pevsCharges;

		for (int pev = 0; pev < nPEVs; pev++)
		{
			vector<bool> pevCharge;
			for (int c = 0; c < nDiscrete; c++)
			{
				//cout<<nV + 1 + pevC + pev * nDiscrete<<endl;
				double boolCharge = vals[nV + 2 + pev * nDiscrete + c];
				if (boolCharge > 0.9)
					pevCharge.push_back(true);
				else
					pevCharge.push_back(false);
			}

			pevsCharges.push_back(pevCharge);

//			for (int c = 0; c < nDiscrete; c++)
//				cout << pevsCharges[pev][c] << "\t";
//			cout << endl;
//			getchar();

		}

		energyCharging.push_back(pevsCharges);

		vector<vector<bool> > pevsDischarge;

		for (int pev = 0; pev < nPEVs; pev++)
		{

			vector<bool> pevDischargeCharge;
			for (int c = 0; c < nDiscrete; c++)
			{
				double boolDischarge = vals[nV + 2 + nPEVs * nDiscrete + pev * nDiscrete + c];
				//cout<<nV + 1 +nPEV*nDiscrete + pevC + pev * nDiscrete<<endl;
				if (boolDischarge > 0.9)
					pevDischargeCharge.push_back(true);
				else
					pevDischargeCharge.push_back(false);
			}
			pevsDischarge.push_back(pevDischargeCharge);

//																for (int c = 0; c < nDiscrete; c++)
//																	cout << pevsDischarge[pev][c] << "\t";
//																cout << endl;
//																getchar();
		}

		energyDischarging.push_back(pevsDischarge);
	}

	batteryRate.resize(nPEVs);
	for (int f = 0; f < fH; f++)
		for (int pev = 0; pev < nPEVs; pev++)
		{
			batteryRate[pev].push_back(vals[nTotal + f * nPEVs + pev]);
			//cout << var[nTotal + f * nPEVs + pev] << " : " << vals[nTotal + f * nPEVs + pev] << endl;
		}

	double totalCharge = vals[nVals - 3];
	double totalDischarge = vals[nVals - 2];
	double totalChargeAndDischarge = vals[nVals - 1];
	cout << var[nVals - 3] << ":" << totalCharge << endl;
	cout << var[nVals - 2] << ":" << totalDischarge << endl;
	cout << var[nVals - 1] << ":" << totalChargeAndDischarge << endl;

	vector<double> otherRelevant =
	{ totalDischarge, totalChargeAndDischarge };
	popOtherRelevantValues.push_back(otherRelevant);
	popGridRate.push_back(gridRate);

//	for (int it = 0; it < fH; it++)
//	{
//		cout << "energySelling[" << it << "] = " << energySelling[it] << endl;
//		cout << "energyBuying[" << it << "] = " << energyBuying[it] << endl;
//		cout << "gridRate[" << it << "] = " << gridRate[it] << endl;
//		cout << "batteryRatePEV1[" << it << "] = " << batteryRate[0][it] << endl;
//		cout << "batteryRatePEV2[" << it << "] = " << batteryRate[1][it] << endl;
//		cout << "batteryRatePEV3[" << it << "] = " << batteryRate[2][it] << endl;
//		getchar();
//	}

	return solObj;
}

void SPOOLStructSmartStorage::exportParetoFrontValues(string filename, int nMILPProblems, int tLim, int nOptObj, double tNow)
{
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

		for (int oR = 0; oR < popOtherRelevantValues[nS].size(); oR++)
		{
			fprintf(fFronteiraPareto, "%.5f\t", popOtherRelevantValues[nS][oR]);
		}

		for (int gR = 0; gR < popGridRate[nS].size(); gR++)
		{
			fprintf(fFronteiraPareto, "%.5f\t", popGridRate[nS][gR]);
		}

		fprintf(fFronteiraPareto, "\n");
	}
	fprintf(fFronteiraPareto, "%.5f \n", tNow);
	fprintf(fFronteiraPareto, "hv: %.5f \n", getParetoHyperVolume());
	fprintf(fFronteiraPareto, "delta: %.5f \n", getParetoDeltaMetric());
	fclose(fFronteiraPareto);
	cout << "File wrote with success!" << endl;
}
