
#include "actasp/AnswerSet.h"
#include <actasp/Action.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace actasp;

//// Code copied from Clingo.coo

class SimpleAction : public actasp::Action {
public:

	SimpleAction() : done(false) {}

	int paramNumber() const {
		return 0;
	}

	std::string getName() const {
		return name;
	}

	void run() {
		std::cout << "running " << name << std::endl;
		done = true;
	}

	bool hasFinished() const {
		return done;
	}

	virtual actasp::Action *cloneAndInit(const actasp::AspFluent &f) const {
		SimpleAction *newAction = new SimpleAction(*this);
		newAction->name = f.getName();
		newAction->params = f.getParameters();
		return newAction;
	}
	
	virtual actasp::Action *clone() const {
		return new SimpleAction(*this);
	}

private:
	std::string name;
	std::vector<std::string> params;
	bool done;

	std::vector<std::string> getParameters() const {
		return params;
	}

};

struct DeleteAction {
	
	void operator()(Action *act) {
		delete act;
	}
};

struct CreateFluent {
	AspFluent operator()(const std::string & fluentDescription) {
		return AspFluent(fluentDescription);
	}
};

static std::vector<actasp::AspFluent> parseAnswerSet(const std::string& answerSetContent) {
	stringstream content(answerSetContent);

	string line;
	getline(content,line); //the first line contains the sequence number of the answer set.
	getline(content,line);

	stringstream predicateLine(line);

	vector<AspFluent> predicates;

	//split the line based on spaces
	transform(istream_iterator<string>(predicateLine),
	          istream_iterator<string>(),
	          back_inserter<vector<AspFluent> >(predicates),
	          CreateFluent());

	return predicates;
}

static std::vector<actasp::AnswerSet> readAnswerSets(istream &input) {
	
	string line;
	string answerSetContent;
	while (input) {
		getline(input, line);
		answerSetContent += line;
		answerSetContent += "\n";
	}

	bool satisfiable = answerSetContent.find("UNSATISFIABLE") == string::npos;

	vector<AnswerSet> allSets;

	if (satisfiable) {

		stringstream content(answerSetContent);

		string firstLine;
		string eachAnswerset;

		while (content) {

			getline(content,firstLine);
			if (firstLine.find("Answer") != string::npos) {
				getline(content,eachAnswerset);
				vector<AspFluent> fluents = parseAnswerSet(eachAnswerset);
				allSets.push_back(AnswerSet(true, fluents));
			}
		}
	}

	return allSets;
	
}

int main() {
	
	std::map<std::string, actasp::Action *> actionMap;
	actionMap.insert(std::make_pair(std::string("north"), new SimpleAction()));
	actionMap.insert(std::make_pair(std::string("south"), new SimpleAction()));
	actionMap.insert(std::make_pair(std::string("east"), new SimpleAction()));
	actionMap.insert(std::make_pair(std::string("west"), new SimpleAction()));
	
	vector<AnswerSet> sets = readAnswerSets(cin);
	
	cout << endl << "Number of answer sets: " << sets.size() << endl;
	
	for(int i=0; i<sets.size(); ++i) {
		
		cout <<  "------------------------------" << endl;
		
		cout << "Plan" << endl;
		
		list<Action*> plan = sets[i].instantiateActions(actionMap);
		list<Action*>::iterator pIt = plan.begin();
		
		for(int t=0; pIt != plan.end(); ++pIt, ++t)
			if ((*pIt) != NULL)
				cout << (*pIt)->toASP(t) << " ";
		
		for_each(plan.begin(),plan.end(),DeleteAction());
		
		cout << endl << endl << "Fluents" << endl;
		
		
		//fluents in an answer set are ordered by time step.
		unsigned int lastTimeStep = sets[i].getFluents().at(sets[i].getFluents().size()-1).getTimeStep();
		
		for(int t = 0; t < lastTimeStep; ++t) {
			
		vector<AspFluent> fluentsAtTimeT = sets[i].getFluentsAtTime(t);
		vector<AspFluent>::const_iterator flu = fluentsAtTimeT.begin();
		
		
		for(; flu != fluentsAtTimeT.end(); ++flu)
			cout << flu->toString() << " ";
		
		cout << endl << endl;
		}
		
		cout << endl;
		
	}
	
	
	
	return 0;
}