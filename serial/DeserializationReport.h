#pragma once
//#include "Serialization.h"
//#include "../ecs/EntityAccess.h"

//class DeserializationReport
//{
//public:
//	~DeserializationReport() = default;
//
//	DeserializationReport(const DeserializationReport&) = delete;
//	DeserializationReport(DeserializationReport&&) = delete;
//
//	DeserializationReport& operator=(const DeserializationReport&) = delete;
//	DeserializationReport& operator=(DeserializationReport&&) = delete;
//
//	template <JsonSerializableComponent T>
//	void PushComponentError(const nlohmann::json::exception& err)
//	{
//		static constexpr std::string_view errFmt =
//			"Could not deserialize component '{}': {}";
//
//		storage_.errors_.push_back(MAKE_ERROR_FMT(
//			errFmt, ComponentTypeToName<T>::value, err.what()
//		));
//	}
//
//	std::vector<Error>&& TakeErrors(EntityPassKey key)
//	{
//		std::vector<Error> temp;
//		temp.swap(errors_);
//
//		return std::move(temp);
//	}
//
//	static DeserializationReport& Get()
//	{
//		static std::unique_ptr<DeserializationReport> instance;
//		if (!instance)
//		{
//			instance = std::unique_ptr<DeserializationReport>{
//				new DeserializationReport{}
//			};
//		}
//
//		return *instance;
//	}
//
//private:
//	DeserializationReport() = default;
//
//	std::vector<Error> errors_;
//};