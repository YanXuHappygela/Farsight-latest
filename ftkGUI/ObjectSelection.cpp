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
#include "ObjectSelection.h"

//Constructor
ObjectSelection::ObjectSelection()
{
	selections.clear();
}

void ObjectSelection::clear(void)
{
	if(selections.size() > 0)
	{
		selections.clear();
		emit changed();
	}
}

bool ObjectSelection::isSelected(long int id)
{
	std::set<long int>::iterator it;
	it=selections.find(id);
	if(it == selections.end())
		return false;
	else
		return true;
}

bool ObjectSelection::add(long int id)
{
	std::set<long int>::iterator it;
	it=selections.find(id);
	if( it == selections.end() )
	{
		selections.insert(id);
		emit changed();
		return true;
	}
	else
		return false;
}

void ObjectSelection::addToMerge(long int id)
{
	selections.insert(id);	
}

bool ObjectSelection::add(std::set<long int> ids)
{
	selections.insert(ids.begin(),ids.end());
	emit changed();
	return true;
}

bool ObjectSelection::remove(long int id)
{
	std::set<long int>::iterator it;
	it=selections.find(id);
	if( it == selections.end() )
		return false;
	else
	{
		selections.erase(it);
		emit changed();
		return true;
	}
	
}

bool ObjectSelection::remove(std::set<long int> ids)
{
	selections.erase(ids.begin(),ids.end());
	emit changed();
	return true;
}

bool ObjectSelection::select(long int id)
{
	std::set<long int>::iterator it;
	it=selections.find(id);
	if( it != selections.end() && selections.size() == 1)	//found it, it's the only 1 selected
	{
		return false;
	}
	//didn't find it so clear and add it
	selections.clear();
	selections.insert(id);
	emit changed();
	return true;
}

void ObjectSelection::selectToMerge(long int id)
{
	selections.clear();
	selections.insert(id);
}

bool ObjectSelection::select(std::set<long int> ids)
{
	selections.clear();
	return add(ids);
}

bool ObjectSelection::toggle(long int id)
{
	std::set<long int>::iterator it;
	it=selections.find(id);
	if( it == selections.end() )
	{
		selections.insert(id);
	}
	else
	{
		selections.erase(it);
	}
	emit changed();
	return true;
}

bool ObjectSelection::toggle(std::set<long int> ids)
{
	std::set<long int>::iterator itToggle;
	std::set<long int>::iterator itCurrent;
	for(itCurrent = ids.begin(); itCurrent != ids.end(); ++itCurrent)
	{
		itToggle = selections.find( (*itCurrent) );
		if( itToggle == selections.end() )
		{
			selections.insert((*itCurrent));
		}
		else
		{
			selections.erase(itToggle);
		}
	}
	emit changed();
	return true;
}

std::set<long int> ObjectSelection::getSelections()
{	
	return selections;
}
void ObjectSelection::SetCurrentTime(int t)
{
	Time = t;
	emit TimeChanged();
}
void ObjectSelection::SelectPoints(std::vector<Point> points)
{
	point_selections.clear();
	point_selections = points;
	emit MultiChanged();
}

void ObjectSelection::SetSampleIndex( std::vector< std::vector<long int> > &sampleIndex)
{
	index.clear();
	for( int i = 0; i < sampleIndex.size(); i++)
	{
		index.push_back(sampleIndex[i]);
	}
	emit thresChanged();
}

void ObjectSelection::GetSampleIndex( std::vector< std::vector<long int> > &sampleIndex)
{
	sampleIndex.clear();
	for( int i = 0; i < index.size(); i++)
	{
		sampleIndex.push_back(index[i]);
	}
}

void ObjectSelection::SetClusterIndex( std::vector< std::vector<long int> > &clusIndex)
{
	indexClus.clear();
	for( int i = 0; i < clusIndex.size(); i++)
	{
		indexClus.push_back(clusIndex[i]);
	}
}

void ObjectSelection::GetClusterIndex( std::vector< std::vector<long int> > &clusIndex)
{
	clusIndex.clear();
	for( int i = 0; i < indexClus.size(); i++)
	{
		clusIndex.push_back(indexClus[i]);
	}
}

int ObjectSelection::DeleteCurrentSelectionInTable()
{
	if( selections.size() > 0)
	{
		emit ItemDeleted();
		return selections.size();
	}
	else
	{
		return 0;
	}
}