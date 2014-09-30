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
#ifndef _OBJECTSELECTION_H
#define _OBJECTSELECTION_H

#include <set>
#include <vector>
#include <QObject>
#include <vnl/vnl_matrix.h>

//****************************************************************************
// A class to keep track of selected objects
//**************************************************************************
class ObjectSelection: public QObject
{
	Q_OBJECT;

public:
	ObjectSelection();

	bool isSelected(long int id);		
	bool add(long int id);
	bool add(std::set<long int> ids);
	void addToMerge(long int id);
	bool select(long int id);
	void selectToMerge(long int id);
	bool select(std::set<long int> ids);
	bool remove(long int id);
	bool remove(std::set<long int> ids);
	bool toggle(long int id);
	bool toggle(std::set<long int> ids);
	void clear(void);
	std::set<long int> getSelections(void);	//Returns the selections
	int DeleteCurrentSelectionInTable();

	// Time Selections: Kymograph and 3D View Editing
	struct Point{int id;int new_id;int time;};
	int GetCurrentTime(){return Time;};
	void SetCurrentTime(int t);
	void SelectPoints(std::vector<Point> points);
	std::vector<Point> * GetSelectedPoints(void){ return &point_selections;};

	void SetSampleIndex( std::vector< std::vector<long int> > &sampleIndex);
	void GetSampleIndex( std::vector< std::vector<long int> > &sampleIndex);
	void SetClusterIndex( std::vector< std::vector<long int> > &clusIndex);
	void GetClusterIndex( std::vector< std::vector<long int> > &clusIndex);

signals:
	void changed();
	void TimeChanged();
	void MultiChanged();
	void thresChanged();
	void ItemDeleted();
	
private:
	std::set<long int> selections;
	vnl_matrix<double> moduleAverage;
	int Time;
	std::vector<Point> point_selections; // see if it needs to be cleared later.

	std::vector< std::vector<long int> > index; /** sample index */
	std::vector< std::vector<long int> > indexClus; /** cluster index */
};


#endif
