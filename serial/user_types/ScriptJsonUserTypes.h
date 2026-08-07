#pragma once
#include "../SerializationConcepts.h"
#include "../../systems/ScriptSystem.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScriptTable::Descriptor, functionNames, scriptSignatures)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScriptDataDescriptor, filepath, tableDescriptor)