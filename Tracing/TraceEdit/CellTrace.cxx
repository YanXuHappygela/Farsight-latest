/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

#include "TraceBit.h"
#include "TraceLine.h"
#include "CellTrace.h"

CellTrace::CellTrace()
{
	this->clearAll();
	CellData = vtkSmartPointer<vtkVariantArray>::New();
}
CellTrace::CellTrace(std::vector<TraceLine*> Segments)
{
	this->clearAll();
	CellData = vtkSmartPointer<vtkVariantArray>::New();
	this->setTraces(Segments);
}
void CellTrace::setTraces(std::vector<TraceLine*> Segments)
{
	this->segments = Segments;
	unsigned int i = 0;
	this->NumSegments = (int) this->segments.size() - 1; //segments[0] is soma
	this->stems = (int) this->segments[0]->GetBranchPointer()->size();
	/*this->prediction = this->segments[0]->getPrediction();
	this->confidence = this->segments[0]->getConfidence();*/
	if (this->stems > 0)
	{
		for ( int j = 0; j < this->stems; j++)
		{
			this->MaxMin(this->segments[0]->GetBranchPointer()->at(j)->GetDistToParent(), 
				this->TotalStemDistance, this->MinStemDistance, this->MaxStemDistance);
			if(this->segments[0]->GetBranchPointer()->at(j)->GetTerminalDegree() > 1)
				this->branchingStem++;

		}
		this->EstimatedSomaRadius = this->TotalStemDistance/(double)this->stems;
	}
	else
	{
		this->MinStemDistance = -PI;
		this->MaxStemDistance = -PI;
		this->TotalStemDistance = -PI;
		this->EstimatedSomaRadius = -PI;
	}
	TraceBit rootBit = this->segments[0]->GetTraceBitsPointer()->front();
	//set the soma point as the intital bounds
	this->somaX = rootBit.x;
	this->somaY = rootBit.y;
	this->somaZ = rootBit.z;
	this->maxX = rootBit.x;
	this->maxY = rootBit.y;
	this->maxZ = rootBit.z;
	this->minX = rootBit.x;
	this->minY = rootBit.y;
	this->minZ = rootBit.z;

	tips.push_back(rootBit);

	this->somaRadii = this->segments[0]->GetRadii();
	this->somaSurface = this->segments[0]->GetSomaSurfaceArea();
	this->somaVolume = this->segments[0]->GetSomaVolume();
	this->DeviceDistance = this->segments[0]->GetDistanceToROI();
	this->DeviceAzimuth = this->segments[0]->GetAzimuthToROI();
	this->DeviceElevation = this->segments[0]->GetElevationToROI();

	for(i = 1; i < this->segments.size(); i++)
	{
		this->IDs.insert(this->segments[i]->GetId());
		this->MaxMin(this->segments[i]->GetLength(), this->PathLengthTotal, this->PathLengthMin, this->PathLengthMax);
		this->MaxMin(this->segments[i]->GetVolume(), this->TotalVolume, this->SegmentVolumeMin, this->SegmentVolumeMax);
		this->MaxMin(this->segments[i]->GetEuclideanLength(),this->TotalEuclideanPath, this->MinEuclideanPath, this->MaxEuclideanPath);
		this->MaxMin(this->segments[i]->GetSurfaceArea(), this->surfaceAreaTotal, this->SurfaceAreaMin, this->SurfaceAreaMax);
		this->MaxMin(this->segments[i]->GetSectionArea(), this->sectionAreaTotal, this->SectionAreaMin, this->SectionAreaMax);
		this->MaxMin(this->segments[i]->GetSize(), this->FragmentationTotal, this->FragmentationMin, this->FragmentationMax);
		this->MaxMin(this->segments[i]->GetBurkTaper(), this->BurkTaperTotal, this->BurkTaperMin, this->BurkTaperMax);
		this->MaxMin(this->segments[i]->GetHillmanTaper(), this->HillmanTaperTotal, this->HillmanTaperMin, this->HillmanTaperMax);

		double diam = 2*this->segments[i]->GetRadii();
		this->MaxMin(diam, this->DiameterTotal, this->DiameterMin, this->DiameterMax);
		double diamPower = pow(diam,1.5);
		this->MaxMin(diamPower, this->DiameterPowerTotal, this->DiameterPowerMin, this->DiameterPowerMax);

		this->MaxMin(this->segments[i]->GetFragmentationSmoothness(), this->ContractionTotal, this->ContractionMin, this->ContractionMax);
		int tempLevel = this->segments[i]->GetLevel();
		if (this->segments[i]->isLeaf())
		{
			this->terminalTips++;

			TraceBit leafBit = this->segments[i]->GetTraceBitsPointer()->back();
			TraceBit leadBit = this->segments[i]->GetTraceBitsPointer()->front();
			this->MaxMin(this->segments[i]->GetSize(), this->TerminalSegmentTotal, this->TerminalSegmentMin, this->TerminalSegmentMax);
			double tipEucDistanceToSoma = this->segments[i]->Euclidean(leafBit, rootBit);
			this->MaxMin(tipEucDistanceToSoma, this->TipToSomaEucDisTotal, this->TipToSomaEucDisMin, this->TipToSomaEucDisMax);
			if(!this->segments[i]->isRoot())
			{
				TraceBit parentBit = this->segments[i]->GetParent(0)->GetTraceBitsPointer()->back();
				double parentDiam = 2*parentBit.r;
				this->TotalLastParentDiam += parentDiam;
				if (this->LastParentDiamMax < parentDiam)
				{
					this->LastParentDiamMax = parentDiam;
				}else if(this->LastParentDiamMin > parentDiam)
				{
					this->LastParentDiamMin = parentDiam;
				}
			}//fails if non branching tree
			double DiamThreshold = 2*leadBit.r;
			this->DiamThresholdTotal += DiamThreshold;
			if (this->DiamThresholdMin > DiamThreshold)
			{
				this->DiamThresholdMin = DiamThreshold;
			}else if(this->DiamThresholdMax < DiamThreshold)
			{
				this->DiamThresholdMax = DiamThreshold;
			}
			float lx = leafBit.x;
			float ly = leafBit.y;
			float lz = leafBit.z;
			float total; //just so things work
			this->MaxMin(lx, total, this->minX, this->maxX);
			this->MaxMin(ly, total, this->minY, this->maxY);
			this->MaxMin(lz, total, this->minZ, this->maxZ);
			this->MaxMin(tempLevel, this->SumTerminalLevel, this->MinTerminalLevel, this->MaxTerminalLevel);
			this->MaxMin(this->segments[i]->GetPathLength(), this->TerminalPathLength, this->TerminalPathLengthMin, this->TerminalPathLengthMax);
			this->totalTipX += (lx - this->somaX);
			this->totalTipY += (ly - this->somaY);
			this->totalTipZ += (lz - this->somaZ);

			tips.push_back(leafBit);
		}//end if leaf
		else if(!this->segments[i]->isRoot())
		{
			if (this->segments[i]->isActualBifurcation())
			{
				this->actualBifurcations++;
			}
			this->branchPoints++;
			TraceBit leadBit = this->segments[i]->GetTraceBitsPointer()->front();
			double BifToSomaEucDistance = this->segments[i]->Euclidean(leadBit,rootBit);
			this->MaxMin(BifToSomaEucDistance, this->BranchPtToSomaEucDisTotal, this->BranchPtToSomaEucDisMin, this->BranchPtToSomaEucDisMax);
			this->MaxMin(this->segments[i]->GetpartitionAsymmetry(), this->partitionAsymmetry, this->partitionAsymmetryMin, this->partitionAsymmetryMax);
			this->MaxMin(this->segments[i]->GetdaughterRatio(), this->daughterRatio, this->daughterRatioMin, this->daughterRatioMax);
			this->MaxMin(this->segments[i]->GetparentDaughterRatio(), this->parentDaughterRatio, this->parentDaughterRatioMin, this->parentDaughterRatioMax);
			this->MaxMin(this->segments[i]->GetdaughterLengthRatio(), this->daughterLengthRatio, this->daughterLengthRatioMin, this->daughterLengthRatioMax);
			this->MaxMin(this->segments[i]->GetRallPower(), this->rallPower, this->rallPowerMin, this->rallPowerMax);
			this->MaxMin(this->segments[i]->GetPk(), this->Pk, this->PkMin, this->PkMax);
			this->MaxMin(this->segments[i]->GetPk_2(), this->Pk_2, this->Pk_2Min, this->Pk_2Max);
			this->MaxMin(this->segments[i]->GetPk_classic(), this->Pk_classic, this->Pk_classicMin, this->Pk_classicMax);
			this->MaxMin(this->segments[i]->GetBifAmplLocal(), this->BifAmplLocal, this->BifAmplLocalMin, this->BifAmplLocalMax);
			this->MaxMin(this->segments[i]->GetBifAmplRemote(), this->BifAmplRemote, this->BifAmplRemoteMin, this->BifAmplRemoteMax);
			this->MaxMin(this->segments[i]->GetBifTiltLocal(), this->BifTiltLocal, this->BifTiltLocalMin, this->BifTiltLocalMax,this->BifTiltLocalCount);
			if(this->segments[i]->isBranch())
			{
				this->MaxMin(this->segments[i]->GetBifTiltRemote(), this->BifTiltRemote, this->BifTiltRemoteMin, this->BifTiltRemoteMax,this->BifTiltRemoteCount);
				this->MaxMin(this->segments[i]->GetBifTorqueLocal(),this->BifTorqueLocal, this->BifTorqueLocalMin, this->BifTorqueLocalMax, this->BifTorqueLocalCount);
				if (this->segments[i]->GetTerminalDegree() > 2)
				{
					this->MaxMin(this->segments[i]->GetBifTorqueRemote(),this->BifTorqueRemote, this->BifTorqueRemoteMin, this->BifTorqueRemoteMax, this->BifTorqueRemoteCount);
				}
			}//end if branch
			if (this->segments[i]->GetLevel() == 1)
			{
				this->MaxMin(this->segments[i]->GetAzimuth(), this->Azimuth, this->AzimuthMin, this->AzimuthMax);
				this->MaxMin(this->segments[i]->GetElevation(), this->Elevation, this->ElevationMin, this->ElevationMax);
			}
			if (this->segments[i]->GetTerminalDegree() ==2)
			{
				this->terminalBifCount++;
				this->MaxMin(this->segments[i]->GetHillmanThreshold(), this->HillmanThreshTotal, this->HillmanThreshMin, this->HillmanThreshMax);
			}
		}//end bifurcation features
	}//end for segment size
	this->skewnessX = (somaX - (minX + (maxX - minX)/2)); /// ((float)(maxX - minX)/2);
	this->skewnessY = (somaY - (minY + (maxY - minY)/2)); /// ((float)(maxY - minY)/2);
	this->skewnessZ = (somaZ - (minZ + (maxZ - minZ)/2)); /// ((float)(maxZ - minZ)/2);
	this->euclideanSkewness = sqrt(pow(skewnessX, 2) + pow(skewnessY, 2) + pow(skewnessZ, 2));

	if (tips.size() > 0)
	{
		this->tipMagnitude = sqrt(pow(totalTipX,2)+pow(totalTipY,2)+pow(totalTipZ,2));
		this->tipAzimuth = atan2(totalTipY,totalTipX)*180/PI;
		double hypotenuse = sqrt(pow(totalTipX,2)+pow(totalTipY,2));
		this->tipElevation = atan2(totalTipZ,hypotenuse)*180/PI;
	}
	this->modified = true;
}
void CellTrace::setFileName(std::string newFileName)
{
	this->FileName = newFileName;
}
std::string CellTrace::GetFileName()
{
	std::string outputFileName;
	if (this->FileName != "file")
	{
		outputFileName = this->FileName;
	}
	else
	{
		std::stringstream newCellName; 
		newCellName<<"cell" << this->segments[0]->GetId();
		outputFileName = newCellName.str();
	}
	return outputFileName;
}
void CellTrace::getSomaCoord(double xyz[])
{
	xyz[0] = this->somaX;
	xyz[1] = this->somaY;
	xyz[2] = this->somaZ;
}
void CellTrace::getCellBounds(double bounds[])
{//min then max of x, y, z
	bounds[0] = this->minX;
	bounds[1] = this->maxX;
	bounds[2] = this->minY;
	bounds[3] = this->maxY;
	bounds[4] = this->minZ;
	bounds[5] = this->maxZ;
}
void CellTrace::setDistanceToROI(double newDistance, double Coord_X , double Coord_Y, double Coord_Z)
{
	this->segments[0]->SetDistanceToROI(newDistance);
	this->segments[0]->SetDistanceToROICoord_X(Coord_X);
	this->segments[0]->SetDistanceToROICoord_Y(Coord_Y);
	this->segments[0]->SetDistanceToROICoord_Z(Coord_Z);

	TraceBit tip;
	tip.x = this->totalTipX;
	tip.y = this->totalTipY;
	tip.z = this->totalTipZ;
	this->segments[0]->CalculateDirectionToROI(tip);

	this->modified = true;
}
void CellTrace::SetClassification(int predicCol, double prediction, int confCol,double confidence)
{
	/**
	*	Needs to be fixed.
	*/
	//this->segments[0]->SetClassification(prediction, confidence); //note: this function should not exist
	//this->segments[0]->editCellFeature(prediction, predicCol);
	//this->segments[0]->editCellFeature(confidence, confCol);
	//this->modified = true;
}
void CellTrace::addNewFeature(std::string featureName, vtkVariant nextFeature)
{
	this->segments[0]->SetCellFeature(featureName,nextFeature);
	this->modified = true;
}
vtkVariant CellTrace::getFeature(std::string featureName)
{
	return this->segments[0]->GetCellFeature(featureName);
}
void CellTrace::clearAll()
{
	this->segments.clear();
	this->NumSegments = 0;
	this->stems = 0;
	this->branchPoints = 0;
	this->terminalTips = 0;
	this->actualBifurcations = 0;
	this->branchingStem = 0;
	//this->TriCount = 0;
	//this->TerminalTriCount = 0;
	//this->notTerminalTriCount = 0;

	this->MinTerminalLevel = 1000; //something large for initial value
	this->MaxTerminalLevel = 0;
	this->SumTerminalLevel = 0;

	this->TerminalPathLengthMin = 1000;
	this->TerminalPathLengthMax = 0;
	this->TerminalPathLength = 0;

	this->FragmentationTotal = 0;
	this->FragmentationMin = 1000;
	this->FragmentationMax = 0;

	this->HillmanTaperTotal = 0;
	this->HillmanTaperMin = 1000;
	this->HillmanTaperMax = 0;

	this->BurkTaperTotal = 0;
	this->BurkTaperMin = 1000;
	this->BurkTaperMax = 0;

	this->HillmanThreshTotal = 0;
	this->HillmanThreshMin = 1000;
	this->HillmanThreshMax = 0;
	this->terminalBifCount = 0;

	this->ContractionTotal = 0;
	this->ContractionMin = 1000;
	this->ContractionMax = 0;

	this->DiameterTotal = 0;
	this->DiameterMin = 1000;
	this->DiameterMax = 0;

	this->DiameterPowerTotal = 0;
	this->DiameterPowerMin = 1000;
	this->DiameterPowerMax = 0;

	this->TotalEuclideanPath = 0;
	this->MinEuclideanPath = 1000;
	this->MaxEuclideanPath = 0;

	this->PathLengthTotal = 0;
	this->PathLengthMin = 1000;
	this->PathLengthMax = 0;

	this->TerminalSegmentTotal = 0;
	this->TerminalSegmentMin = 1000;
	this->TerminalSegmentMax = 0;

	this->TotalVolume = 0;
	this->SegmentVolumeMax = 0;
	this->SegmentVolumeMin = 1000;

	this->FileName = "file";

	this->somaX = 0;
	this->somaY = 0;
	this->somaZ = 0;

	this->maxX = 0;
	this->maxY = 0;
	this->maxZ = 0;

	this->minX = 100;
	this->minY = 100;
	this->minZ = 100;

	this->sectionAreaTotal = 0;
	this->SectionAreaMax = 0;
	this->SectionAreaMin = 1000;

	this->surfaceAreaTotal = 0;
	this->SurfaceAreaMax = 0;
	this->SurfaceAreaMin = 1000;

	this->DiamThresholdTotal = 0;
	this->DiamThresholdMax = 0;
	this->DiamThresholdMin = 1000;

	this->TotalLastParentDiam = 0;
	this->LastParentDiamMax = 0;
	this->LastParentDiamMin = 1000;

	this->somaVolume= 0;
	this->somaSurface = 0;
	this->somaRadii = 0;

	this->MinStemDistance = 1000;
	this->TotalStemDistance = 0;
	this->MaxStemDistance = 0; 
	this->EstimatedSomaRadius = -PI;

	this->daughterRatio = 0;
	this->daughterRatioMin = 1000;
	this->daughterRatioMax = 0;

	this->parentDaughterRatio = 0;
	this->parentDaughterRatioMin = 1000;
	this->parentDaughterRatioMax = 0;

	this->daughterLengthRatio = 0;
	this->daughterLengthRatioMin = 1000;
	this->daughterLengthRatioMax = 0;

	this->partitionAsymmetry = 0;
	this->partitionAsymmetryMin = 1000;
	this->partitionAsymmetryMax = 0;

	this->rallPower = 0;
	this->rallPowerMin = 100;
	this->rallPowerMax = 0;

	this->Pk = 0;
	this->PkMin = 100;
	this->PkMax = 0;

	this->Pk_2 = 0;
	this->Pk_2Min = 100;
	this->Pk_2Max = 0;

	this->Pk_classic = 0;
	this->Pk_classicMin = 100;
	this->Pk_classicMax = 0;

	this->BifAmplLocal = 0;
	this->BifAmplLocalMin = 180;
	this->BifAmplLocalMax = 0;

	this->BifAmplRemote = 0;
	this->BifAmplRemoteMin = 180;
	this->BifAmplRemoteMax = 0;

	this->BifTiltLocal = 0;
	this->BifTiltLocalMin = 180;
	this->BifTiltLocalMax = 0;
	this->BifTiltLocalCount = 0;

	this->BifTiltRemote = 0;
	this->BifTiltRemoteMin = 180;
	this->BifTiltRemoteMax = 0;
	this->BifTiltRemoteCount = 0;

	this->BifTorqueLocal = 0;
	this->BifTorqueLocalMin = 180;
	this->BifTorqueLocalMax = 0;
	this->BifTorqueLocalCount = 0;
	this->BifTorqueRemote = 0;
	this->BifTorqueRemoteMin = 180;
	this->BifTorqueRemoteMax = 0;
	this->BifTorqueRemoteCount = 0;

	this->Azimuth = 0;
	this->AzimuthMin = 180;
	this->AzimuthMax = 0;

	this->Elevation = 0;
	this->ElevationMin = 180;
	this->ElevationMax = 0;

	this->TipToSomaEucDisTotal = 0;
	this->TipToSomaEucDisMin = 1000;
	this->TipToSomaEucDisMax = 0;

	this->BranchPtToSomaEucDisTotal = 0;
	this->BranchPtToSomaEucDisMin = 1000;
	this->BranchPtToSomaEucDisMax = 0;

	this->totalTipX = 0;
	this->totalTipY = 0;
	this->totalTipZ = 0;

	this->tipMagnitude = -1000;
	this->tipAzimuth = -1000;
	this->tipElevation = -1000;

	this->DeviceDistance = 0;
	this->prediction = -PI;
	this->confidence = -PI;
	this->DeviceAzimuth = 0;
	this->DeviceElevation = 0;
	this->cellDirectiontoDevice = 0;

	//change to -PI later
	this->convexHullMagnitude = -1000;
	this->convexHullAzimuth = -1000;
	this->convexHullElevation = -1000;
	this->convexHullArea = -1000;
	this->convexHullVol = -1000;

	this->modified = false;
	this->delaunayCreated = false;
}
void CellTrace::MaxMin(double NewValue, double &total, double &Min, double &Max)
{
	if (NewValue != -1)
	{
		total += NewValue;
		if (NewValue > Max)
		{
			Max = NewValue;
		}
		if (NewValue < Min)
		{
			Min = NewValue;
		}
	}
}
void CellTrace::MaxMin(double NewValue, double &total, double &Min, double &Max, int &Count)
{
	if (NewValue != -1)
	{
		this->MaxMin(NewValue,total,Min,Max);
		Count++;
	}
}
void CellTrace::MaxMin(float NewValue, float &total, float &Min, float &Max)
{
	if (NewValue != -1)
	{
		total += NewValue;
		if (NewValue > Max)
		{
			Max = NewValue;
		}
		if (NewValue < Min)
		{
			Min = NewValue;
		}
	}
}
void CellTrace::MaxMin(int NewValue, int &total, int &Min, int &Max)
{
	if (NewValue != -1)
	{
		total += NewValue;
		if (NewValue > Max)
		{
			Max = NewValue;
		}
		if (NewValue < Min)
		{
			Min = NewValue;
		}
	}
}
void CellTrace::MaxMin(std::vector<double> NewValue, double &total, double &Min, double &Max, int &Count)
{
	for (int i = 0; i < NewValue.size(); i++)
	{
		if (NewValue[i] != -1)
		{
			total += NewValue[i];
			if (NewValue[i] > Max)
			{
				Max = NewValue[i];
			}
			if (NewValue[i] < Min)
			{
				Min = NewValue[i];
			}
			Count++;
		}
	}
}
vtkSmartPointer<vtkVariantArray> CellTrace::DataRow()
{
	if (this->modified)
	{
		CellData->Reset();
		CellData->InsertNextValue(this->segments[0]->GetId());

		CellData->InsertNextValue(this->somaX);
		CellData->InsertNextValue(this->somaY);
		CellData->InsertNextValue(this->somaZ);

		//CellData->InsertNextValue(this->maxX - this->minX);//Width
		//CellData->InsertNextValue(this->maxY - this->minY);//Length
		//CellData->InsertNextValue(this->maxZ - this->minZ);//Height		

		//CellData->InsertNextValue(this->somaRadii);
		//CellData->InsertNextValue(this->somaSurface);
		//CellData->InsertNextValue(this->somaVolume);

		CellData->InsertNextValue(this->skewnessX);
		CellData->InsertNextValue(this->skewnessY);
		CellData->InsertNextValue(this->skewnessZ);
		CellData->InsertNextValue(this->euclideanSkewness);

		CellData->InsertNextValue(this->NumSegments);
		CellData->InsertNextValue(this->stems);
		CellData->InsertNextValue(this->branchingStem);
		CellData->InsertNextValue(this->branchPoints);
		CellData->InsertNextValue(this->actualBifurcations);
		CellData->InsertNextValue(this->terminalTips);

		//protect from divide by zero
		int tempNumSegments = 1;
		if (this->NumSegments != 0)
		{
			tempNumSegments = this->NumSegments;
		}
		int tempBranchPoints = 1;
		if (this->branchPoints != 0)
		{
			tempBranchPoints = this->branchPoints;
		}
		int tempActualBifurcations = 1;
		if (this->actualBifurcations != 0)
		{
			tempActualBifurcations = this->actualBifurcations;
		}

		CellData->InsertNextValue(this->DiameterMin);
		CellData->InsertNextValue(this->DiameterTotal / tempNumSegments);
		CellData->InsertNextValue(this->DiameterMax);

		CellData->InsertNextValue(this->DiameterPowerMin);
		CellData->InsertNextValue(this->DiameterPowerTotal / tempNumSegments);
		CellData->InsertNextValue(this->DiameterPowerMax);

		CellData->InsertNextValue(this->TotalVolume);
		CellData->InsertNextValue(this->SegmentVolumeMin);
		CellData->InsertNextValue(this->TotalVolume/tempNumSegments);////average segment Volume
		CellData->InsertNextValue(this->SegmentVolumeMax);
		CellData->InsertNextValue(this->surfaceAreaTotal);
		CellData->InsertNextValue(this->SurfaceAreaMin);
		CellData->InsertNextValue(this->surfaceAreaTotal/tempNumSegments);
		CellData->InsertNextValue(this->SurfaceAreaMax);
		//CellData->InsertNextValue(this->sectionAreaTotal);
		CellData->InsertNextValue(this->SectionAreaMin);
		CellData->InsertNextValue(this->sectionAreaTotal/tempNumSegments);
		CellData->InsertNextValue(this->SectionAreaMax);

		//CellData->InsertNextValue(this->BurkTaperTotal);
		CellData->InsertNextValue(this->BurkTaperMin);
		CellData->InsertNextValue(this->BurkTaperTotal / (tempActualBifurcations*2));
		CellData->InsertNextValue(this->BurkTaperMax);

		//CellData->InsertNextValue(this->HillmanTaperTotal);
		CellData->InsertNextValue(this->HillmanTaperMin);
		CellData->InsertNextValue(this->HillmanTaperTotal / (tempActualBifurcations*2));
		CellData->InsertNextValue(this->HillmanTaperMax);

		CellData->InsertNextValue(this->TotalEuclideanPath);
		CellData->InsertNextValue(this->TotalEuclideanPath/tempNumSegments);//average segment euclidean length
		CellData->InsertNextValue(this->PathLengthTotal);
		CellData->InsertNextValue(this->PathLengthMin);
		CellData->InsertNextValue(this->PathLengthTotal/tempNumSegments);//average segment length
		CellData->InsertNextValue(this->PathLengthMax);

		CellData->InsertNextValue(this->MinStemDistance);
		CellData->InsertNextValue(this->EstimatedSomaRadius);
		CellData->InsertNextValue(this->MaxStemDistance);

		CellData->InsertNextValue(this->ContractionMin);
		double aveContraction = this->ContractionTotal / tempNumSegments;
		if (aveContraction != aveContraction)
		{
			CellData->InsertNextValue(-PI);
		}
		else
		{
			CellData->InsertNextValue(aveContraction);
		}
		CellData->InsertNextValue(this->ContractionMax);

		CellData->InsertNextValue(this->FragmentationTotal);
		CellData->InsertNextValue(this->FragmentationMin);
		CellData->InsertNextValue((int) floor((double) this->FragmentationTotal / tempNumSegments + 0.5));
		CellData->InsertNextValue(this->FragmentationMax);

		CellData->InsertNextValue(this->daughterRatioMin);
		CellData->InsertNextValue(this->daughterRatio / tempActualBifurcations);
		CellData->InsertNextValue(this->daughterRatioMax);

		CellData->InsertNextValue(this->parentDaughterRatioMin);
		CellData->InsertNextValue(this->parentDaughterRatio/ tempActualBifurcations);
		CellData->InsertNextValue(this->parentDaughterRatioMax);

		CellData->InsertNextValue(this->daughterLengthRatioMin);
		CellData->InsertNextValue(this->daughterLengthRatio/ tempActualBifurcations);
		CellData->InsertNextValue(this->daughterLengthRatioMax);

		CellData->InsertNextValue(this->partitionAsymmetryMin);
		CellData->InsertNextValue(this->partitionAsymmetry / tempActualBifurcations);
		CellData->InsertNextValue(this->partitionAsymmetryMax);

		CellData->InsertNextValue(this->rallPowerMin);
		CellData->InsertNextValue(this->rallPower / tempActualBifurcations);
		CellData->InsertNextValue(this->rallPowerMax);

		CellData->InsertNextValue(this->PkMin);
		CellData->InsertNextValue(this->Pk / tempActualBifurcations);
		CellData->InsertNextValue(this->PkMax);

		CellData->InsertNextValue(this->Pk_classicMin);
		CellData->InsertNextValue(this->Pk_classic / tempActualBifurcations);
		CellData->InsertNextValue(this->Pk_classicMax);

		CellData->InsertNextValue(this->Pk_2Min);
		CellData->InsertNextValue(this->Pk_2 / tempActualBifurcations);
		CellData->InsertNextValue(this->Pk_2Max);

		double AveAzimuth = -PI;
		double AveElevation = -PI;
		if (this->stems !=0)
		{
			AveAzimuth = this->Azimuth / this->stems;
			AveElevation = this->Elevation / this->stems;
		}
		CellData->InsertNextValue(this->AzimuthMin);
		CellData->InsertNextValue(AveAzimuth);
		CellData->InsertNextValue(this->AzimuthMax);
		CellData->InsertNextValue(this->ElevationMin);
		CellData->InsertNextValue(AveElevation);
		CellData->InsertNextValue(this->ElevationMax);

		if (this->BifTorqueLocalCount == 0)
		{
			this->BifTorqueLocalCount = 1;
		}
		if (this->BifTorqueRemoteCount == 0)
		{
			this->BifTorqueRemoteCount = 1;
		}
		if (this->BifTiltLocalCount == 0)
		{
			this->BifTiltLocalCount = 1;
		}
		if (this->BifTiltRemoteCount == 0)
		{
			this->BifTiltRemoteCount = 1;
		}
		CellData->InsertNextValue(this->BifAmplLocalMin);
		CellData->InsertNextValue(this->BifAmplLocal / tempActualBifurcations);
		CellData->InsertNextValue(this->BifAmplLocalMax);
		CellData->InsertNextValue(this->BifTiltLocalMin);
		CellData->InsertNextValue(this->BifTiltLocal / this->BifTiltLocalCount);
		CellData->InsertNextValue(this->BifTiltLocalMax);
		CellData->InsertNextValue(this->BifTorqueLocalMin);
		CellData->InsertNextValue(this->BifTorqueLocal/ this->BifTorqueLocalCount);
		CellData->InsertNextValue(this->BifTorqueLocalMax);

		CellData->InsertNextValue(this->BifAmplRemoteMin);
		CellData->InsertNextValue(this->BifAmplRemote / tempActualBifurcations);
		CellData->InsertNextValue(this->BifAmplRemoteMax);
		CellData->InsertNextValue(this->BifTiltRemoteMin);
		CellData->InsertNextValue(this->BifTiltRemote / this->BifTiltRemoteCount);
		CellData->InsertNextValue(this->BifTiltRemoteMax);
		CellData->InsertNextValue(this->BifTorqueRemoteMin);
		CellData->InsertNextValue(this->BifTorqueRemote/ this->BifTorqueRemoteCount);
		CellData->InsertNextValue(this->BifTorqueRemoteMax);

		int tempTerminalTips = 1;
		if (this->terminalTips != 0)
		{
			tempTerminalTips = this->terminalTips;
		}
		CellData->InsertNextValue(this->MinTerminalLevel);
		CellData->InsertNextValue(this->TerminalPathLengthMin);
		CellData->InsertNextValue(this->SumTerminalLevel /tempTerminalTips);//average terminal level
		CellData->InsertNextValue(this->TerminalPathLength/tempTerminalTips);//now average path to end
		CellData->InsertNextValue(this->MaxTerminalLevel);
		CellData->InsertNextValue(this->TerminalPathLengthMax);

		CellData->InsertNextValue(this->TerminalSegmentTotal);
		CellData->InsertNextValue(this->TerminalSegmentMin);
		CellData->InsertNextValue(this->TerminalSegmentTotal / tempTerminalTips); //average
		CellData->InsertNextValue(this->TerminalSegmentMax);

		CellData->InsertNextValue(this->DiamThresholdMin);
		CellData->InsertNextValue(this->DiamThresholdTotal/tempTerminalTips);
		CellData->InsertNextValue(this->DiamThresholdMax);
		CellData->InsertNextValue(this->LastParentDiamMin);
		CellData->InsertNextValue(this->TotalLastParentDiam/tempTerminalTips);
		CellData->InsertNextValue(this->LastParentDiamMax);

		CellData->InsertNextValue(this->HillmanThreshMin); 
		if (this->terminalBifCount != 0)
		{
			CellData->InsertNextValue(this->HillmanThreshTotal/ this->terminalBifCount);
		}else
		{
			CellData->InsertNextValue(-PI);
		}
		CellData->InsertNextValue(this->HillmanThreshMax);

		CellData->InsertNextValue(this->BranchPtToSomaEucDisMin);
		CellData->InsertNextValue(this->BranchPtToSomaEucDisTotal/tempBranchPoints);
		CellData->InsertNextValue(this->BranchPtToSomaEucDisMax);
		CellData->InsertNextValue(this->TipToSomaEucDisMin);
		CellData->InsertNextValue(this->TipToSomaEucDisTotal/tempTerminalTips);
		CellData->InsertNextValue(this->TipToSomaEucDisMax);

		CellData->InsertNextValue(this->tipMagnitude);
		CellData->InsertNextValue(this->tipAzimuth);
		CellData->InsertNextValue(this->tipElevation);

		//CellData->InsertNextValue(this->GetFileName().c_str());
		/*CellData->InsertNextValue(this->prediction);
		CellData->InsertNextValue(this->confidence);*/
		//CellData->InsertNextValue( this->segments[0]->GetDistanceToROI());
		//std::cout << this->FileName << std::endl;
		this->modified = false;
	}
	return CellData;
}

vtkSmartPointer<vtkVariantArray> CellTrace::GetExtendedDataRow(std::vector<std::string> FeatureNames)
{
	/**
	* needs to get data from root traceline
	*/
	if (this->modified)
	{
		this->DataRow();
		std::vector<std::string>::iterator iter;
		for (iter = FeatureNames.begin(); iter < FeatureNames.end(); iter++)
		{
			vtkVariant extendedFeature = this->segments[0]->GetCellFeature(*iter);
			CellData->InsertNextValue(extendedFeature);
		}
		//std::cout << "row size " << this->CellData->GetNumberOfValues()<< std::endl;
	}
	return this->CellData;
}
vtkSmartPointer<vtkVariantArray> CellTrace::BoundsRow()
{
	// id, somaX somaY somaZ min/max xyz, skewness xyz
	vtkSmartPointer<vtkVariantArray> CellBoundsRow = vtkSmartPointer<vtkVariantArray>::New();
	CellBoundsRow->InsertNextValue(this->segments[0]->GetId());
	CellBoundsRow->InsertNextValue(this->somaX);
	CellBoundsRow->InsertNextValue(this->somaY);
	CellBoundsRow->InsertNextValue(this->somaZ);
	CellBoundsRow->InsertNextValue(this->minX);
	CellBoundsRow->InsertNextValue(this->maxX);
	CellBoundsRow->InsertNextValue(this->minY);
	CellBoundsRow->InsertNextValue(this->maxY);
	CellBoundsRow->InsertNextValue(this->minZ);
	CellBoundsRow->InsertNextValue(this->maxZ);
	return CellBoundsRow;
}
std::string CellTrace::BasicFeatureString()
{
	std::stringstream features;
	features << this->GetFileName().c_str() << "\t";
	features << this->somaX << "\t";
	features << this->somaY << "\t";
	features << this->somaZ << "\t";
	return features.str();
}
std::set<long int> CellTrace::TraceIDsInCell()
{
	return this->IDs;
}
unsigned int CellTrace::rootID()
{
	return this->segments[0]->GetId();
}

TraceLine * CellTrace::getRootTrace()
{
	return this->segments.front();
}

std::vector<TraceLine *> CellTrace::getSegments()
{
	return this->segments;
}

std::vector<std::string> CellTrace::calculateConvexHull()
{
	std::vector<std::string> convexHullHeaders;
	if(!delaunayCreated)
	{
		double point[3];
		point[0] = this->somaX;
		point[1] = this->somaY;
		point[2] = this->somaZ;

		ConvexHull3D * convexHull = new ConvexHull3D();
		convexHull->setPoints(tips);
		convexHull->setReferencePt(point);
		convexHull->calculate();
		convexHull->calculateEllipsoid();

		delaunayActor = convexHull->getActor();
		ellipsoidActor = convexHull->get3DEllipseActor();

		convexHullHeaders = convexHull->getConvexHullHeaders();
		double* convexHullValues = convexHull->getConvexHullValues();

		int convexHullValueIndex = 0;
		std::vector<std::string>::iterator headerIter;
		for (headerIter = convexHullHeaders.begin(); headerIter != convexHullHeaders.end(); headerIter++)
		{
			this->addNewFeature(*headerIter, convexHullValues[convexHullValueIndex]);
			convexHullValueIndex++;
		}
		delaunayCreated = true;
	}
	return convexHullHeaders;
}

vtkSmartPointer<vtkActor> CellTrace::GetDelaunayActor()
{
	if(!delaunayCreated)
	{
		this->calculateConvexHull();
	}
	return delaunayActor;
}

vtkSmartPointer<vtkActor> CellTrace::GetEllipsoidActor()
{
	if(!delaunayCreated)
	{
		this->calculateConvexHull();
	}
	return ellipsoidActor;
}

std::vector<std::string> CellTrace::calculateAnglesToDevice()
{
	std::vector<std::string> DeviceAngleHeaders;
	DeviceAngleHeaders.push_back("Azimuth to Device");
	DeviceAngleHeaders.push_back("Elevation to Device");
	DeviceAngleHeaders.push_back("Cell Tips to Device Angle");

	this->addNewFeature(DeviceAngleHeaders[0],this->segments[0]->GetAzimuthToROI());
	this->addNewFeature(DeviceAngleHeaders[1],this->segments[0]->GetElevationToROI());
	this->addNewFeature(DeviceAngleHeaders[2],this->segments[0]->GetTipToROI());

	return DeviceAngleHeaders;
}

std::string CellTrace::calculateDistanceToVessel(FloatImageType::Pointer distanceMap, ImageType::RegionType region)
{
	itk::Index<3> somaIndex;
	somaIndex[0] = this->somaX;
	somaIndex[1] = this->somaY;
	somaIndex[2] = this->somaZ;

	std::string vesselHeader = "Distance to Vessel";
	if (region.IsInside(somaIndex))
	{
		distanceMap->GetPixel(somaIndex);
		this->addNewFeature(vesselHeader,distanceMap->GetPixel(somaIndex));
	}
	else
	{
		std::cout << this->somaX << "," << this->somaY << "," << this->somaZ << " Cell is outside of the vessel image... skipped" << std::endl;
		this->addNewFeature(vesselHeader,-1);
	}

	return vesselHeader;
}