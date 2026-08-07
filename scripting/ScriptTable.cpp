#include "ScriptTable.h"

bool ScriptTable::RegisterFunction(std::string_view fnName, const ScriptSignature& scriptSig)
{
    if (registeredSignatures_.contains(fnName))
    {
        return false;
    }

    if (!ValidateFunction(fnName))
    {
        return false;
    }

    return registeredSignatures_.emplace(fnName, scriptSig).second;
}

ScriptTableView ScriptTable::GetView() const
{
    return ScriptTableView{ *this };
}

ScriptTable::Descriptor ScriptTable::ExportTableDescriptor() const
{
    Descriptor descriptor{};
    descriptor.functionNames.reserve(registeredSignatures_.size());
    descriptor.scriptSignatures.reserve(registeredSignatures_.size());

    for (const auto& [name, sig] : registeredSignatures_)
    {
        descriptor.functionNames.emplace_back(name);
        descriptor.scriptSignatures.emplace_back(sig.Bits());
    }

    return descriptor;
}
