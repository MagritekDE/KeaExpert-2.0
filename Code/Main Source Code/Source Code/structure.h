#ifndef STRUCTURE_H
#define STRUCTURE_H

class Interface;
class Variable;

namespace Structure{
	enum errorMode {REPORT, NO_REPORT};
}

short EvaluateStructureMember(Interface *itfc, char *operand, Variable *result);
short AssignStructureMember(Interface *itfc, char *dstName, Variable* srcVar);
int MakeStructure(Interface* itfc, char args[]);
int AssignStructure(Interface* itfc, char args[]);
Variable* GetStructureVariable(Interface *itfc, short scope, char *varName, Structure::errorMode report);
Variable* JoinStructures(Variable* varA, Variable* varB);
int MakeParameterStructure(Interface* itfc ,char arg[]);

#endif //define STRUCTURE_H
