#include <MOHPC/Utility/NullRemoteId.h>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(NullRemoteIdentifier);

str NullRemoteIdentifier::getString() const
{
	return "";
}

bool NullRemoteIdentifier::isIdentifier(const IRemoteIdentifier& other) const
{
	return dynamic_cast<const NullRemoteIdentifier*>(&other) == this;
}
