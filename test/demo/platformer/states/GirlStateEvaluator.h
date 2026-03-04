//#pragma once
//#include "../Common.h"
//
//namespace test {
//
//using StateEvalFn = bool(*)(const GirlStateContext&);
//
//template <typename Fn>
//concept SomeStateEvalFn = std::convertible_to<Fn, StateEvalFn>;
//
//class GirlStateEvaluator
//{
//public:
//	enum Operand : uint8_t
//	{
//		And = 1 << 0,
//		Or = 1 << 1,
//		Not = 1 << 2,
//		If = 1 << 3,
//		AndNot = (And | Not),
//		OrNot = (Or | Not),
//		AndIfNot = (If | AndNot),
//		OrIfNot = (If | OrNot),
//	};
//
//	struct Condition
//	{
//		template <SomeStateEvalFn Fn>
//		Condition& And(Fn&& fn);
//
//		template <SomeStateEvalFn Fn>
//		Condition& Or(Fn&& fn);
//
//		template <SomeStateEvalFn Fn>
//		Condition AndIf(Fn&& fn);
//
//		template <SomeStateEvalFn Fn>
//		Condition OrIf(Fn&& fn);
//
//		Condition Close();
//
//		Condition operator()();
//
//	private:
//		Condition(GirlStateEvaluator& p, size_t lvl) : parent(p), nestLevel(lvl) {}
// 
//		GirlStateEvaluator& parent;
//		size_t nestLevel = 0;
//	};
//	friend struct Condition;
//
//	struct EvalStep
//	{
//		uint8_t op;
//		StateEvalFn evalFn = nullptr;
//		size_t compoundId = 0;
//		std::optional<bool> result;
//		size_t nestLevel = 0;
//	};
//
//	struct ExpressionNode
//	{
//	public:
//		StateEvalFn fn = nullptr;
//		Operand op;
//		ExpressionNode* lhs = nullptr;
//		ExpressionNode* rhs = nullptr;
//
//
//		ExpressionNode And()
//
//		constexpr bool IsLeaf() const noexcept { return fn != nullptr; }
//
//		bool Evaluate(const GirlStateContext& ctx);
//	};
//
//	using ExpressionNodePool = std::vector<ExpressionNode>;
//
//	/*template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& And(Fn&& fn)
//	{
//		AddEvalStep(Operand::And, std::forward<Fn>(fn));
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& And()
//	{
//		AddEvalStep(Operand::And, ptr);
//		return *this;
//	}
//
//	template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& AndNot(Fn&& fn)
//	{
//		AddEvalStep((Operand::And | Operand::Not), std::forward<Fn>(fn));
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& AndNot()
//	{
//		AddEvalStep((Operand::And | Operand::Not), ptr);
//		return *this;
//	}
//
//	template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& Or(Fn&& fn)
//	{
//		AddEvalStep(Operand::Or, std::forward<Fn>(fn));
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& Or()
//	{
//		AddEvalStep(Operand::Or, ptr);
//		return *this;
//	}
//
//	template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& OrNot(Fn&& fn)
//	{
//		AddEvalStep((Operand::And | Operand::Not), std::forward<Fn>(fn));
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& OrNot()
//	{
//		AddEvalStep((Operand::And | Operand::Not), ptr);
//		return *this;
//	}
//
//	template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& AndIf(Fn&& fn)
//	{
//		AddEvalStep(Operand::And, std::forward<Fn>(fn), true);
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& AndIf()
//	{
//		AddEvalStep(Operand::And, ptr, true);
//		return *this;
//	}
//
//
//	template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& AndIfNot(Fn&& fn)
//	{
//		AddEvalStep((Operand::And | Operand::Not), std::forward<Fn>(fn), true);
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& AndIfNot()
//	{
//		AddEvalStep((Operand::And | Operand::Not), ptr, true);
//		return *this;
//	}
//
//	template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& OrIf(Fn&& fn)
//	{
//		AddEvalStep(Operand::Or, std::forward<Fn>(fn), true);
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& OrIf()
//	{
//		AddEvalStep(Operand::Or, ptr, true);
//		return *this;
//	}
//
//	template <SomeStateEvalFn Fn>
//	GirlStateEvaluator& OrIfNot(Fn&& fn)
//	{
//		AddEvalStep((Operand::Or | Operand::Not), std::forward<Fn>(fn), true);
//		return *this;
//	}
//	template <auto ptr> requires SomeStateEvalFn<decltype(ptr)>
//	GirlStateEvaluator& OrIfNot()
//	{
//		AddEvalStep((Operand::Or | Operand::Not), ptr, true);
//		return *this;
//	}*/
//
//	bool EvalImpl(EvalStep& lhs, EvalStep& rhs, const GirlStateContext& ctx)
//	{
//		assert(lhs.evalFn);
//		bool lhsResult = std::invoke(lhs.evalFn, ctx);
//	}
//
//	bool Evaluate(const GirlStateContext& ctx)
//	{
//		if (evalSteps_.empty())
//		{
//			return false;
//		}
//
//		size_t currentNestLevel = 0;
//		size_t currentStepIdx = 0;
//		size_t nestStepStartIdx = 0;
//
//		std::vector<bool> nestResults(1ULL);
//
//		auto getCurrentResult = [&] { return nestResults[currentNestLevel]; };
//
//		for (; currentStepIdx < evalSteps_.size(); ++currentStepIdx)
//		{
//			auto& currentStep = evalSteps_[currentStepIdx];
//			assert(currentStep.evalFn);
//
//			bool stepResult = std::invoke(currentStep.evalFn, ctx);
//			if (currentStep.op & Operand::Not)
//			{
//				stepResult = !stepResult;
//			}
//
//			if (currentStep.nestLevel != currentNestLevel)
//			{
//				// starting new nest either deeper or shallower
//				auto& prevNestStep = evalSteps_[nestStepStartIdx];
//
//				bool prevNestResult = nestResults[nestStepStartIdx];
//
//				if (prevNestStep.op & Operand::And)
//				{
//					prevNestResult &= stepResult;
//				}
//				else if (currentStep.op & Operand::Or)
//				{
//					prevNestResult |= stepResult;
//				}
//
//				currentNestLevel = currentStep.nestLevel;
//				nestStepStartIdx = currentStepIdx;
//			}
//
//			if (currentStepIdx < evalSteps_.size() + 1)
//			{
//				auto& nextStep = evalSteps_[currentStepIdx + 1];
//
//				if (nextStep.nestLevel < currentStep.nestLevel)
//				{
//
//				}
//			}
//
//			currentNestLevel = currentStep.nestLevel;
//			++currentStepIdx;
//		}
//
//		//// resolve all individual steps
//		//for (auto& step : evalSteps_)
//		//{
//		//	if (!step.evalFn)
//		//	{
//		//		continue;
//		//	}
//
//		//	step.result = std::invoke(step.evalFn, ctx);
//
//		//	if (step.op & Operand::Not)
//		//	{
//		//		step.result = !step.result;
//		//	}
//		//}
//
//		//// resolve compound statements
//		//size_t currentCompoundId = 0;
//		//size_t sentinelCompoundIdStart = 0;
//		//size_t evalStepIdx = 0;
//
//		//bool finalResult = false;
//		//bool runningResult = false;
//
//		////do
//		////{
//		////	auto& currentStep = evalSteps_[evalStepIdx];
//
//
//
//		////} while (currentCompoundId != lastCompoundId && 
//		////		 evalStepIdx < evalSteps_.size());
//
//		//for (size_t i = 0; i < evalSteps_.size(); ++i)
//		//{
//		//	auto& currentStep = evalSteps_[i];
//		//	bool currentDiscreteResult = discreteEvalResults_[currentCompoundId];
//
//		//	const auto currentOp = currentStep.op;
//
//		//	if (currentStep.compoundId != currentCompoundId)
//		//	{
//		//		assert(currentStep.compoundId == currentCompoundId + 1);
//
//		//		// resolve
//		//		// if (a() && b()) || (c() || d())
//		//		// If(a()).And(b()).OrIf(c()).Or(d()))
//		//		// 1 = true
//		//		// 2
//
//		//		if (i < evalSteps_.size() - 1 &&
//		//			(evalSteps_[i + 1].op & Operand::And) &&
//		//			!runningResult)
//		//		{
//		//			
//		//		}
//		//	}
//
//		//	if (currentStep.result.has_value())
//		//	{
//		//		bool stepResult = std::invoke(currentStep.evalFn, ctx);
//		//		if (currentStep.op & Operand::Not)
//		//		{
//		//			stepResult = !stepResult;
//		//		}
//		//		
//		//		if (currentStep.op & Operand::And)
//		//		{
//		//			currentDiscreteResult &= stepResult;
//		//		}
//		//		else if (currentStep.op & Operand::Or)
//		//		{
//		//			currentDiscreteResult |= stepResult;
//		//		}
//		//	}
//
//		//	discreteEvalResults_[currentCompoundId] = currentDiscreteResult;
//		//}
//
//
//		//bool result = false;
//		//size_t evalStepIdx = 0;
//
//		//size_t currentCompoundId = 0;
//		//size_t newCompoundId = 0;
//
//		//for (auto& step : evalSteps_)
//		//{
//		//	if (step.compoundId != currentCompoundId)
//		//	{
//
//		//	}
//
//		//	if (!step.evalFn)
//		//	{
//		//		++evalStepIdx;
//		//		continue;
//		//	}
//
//		//	bool stepResult = std::invoke(step.evalFn, ctx);
//		//	if (step.op & Operand::Not)
//		//	{
//		//		stepResult = !stepResult;
//		//	}
//
//		//	if (step.op & Operand::And)
//		//	{
//		//		result &= stepResult;
//		//	}
//		//	else if (step.op & Operand::Or)
//		//	{
//		//		result |= stepResult;
//		//	}
//
//
//		return true;
//	}
//
//	bool ResolveCompoundSteps(Range<size_t> compoundRange)
//	{
//
//	}
//
//private:
//	template <SomeStateEvalFn Fn>
//	void AddEvalStep(uint8_t op, Fn&& fn, bool newId = false)
//	{
//		if (newId)
//		{
//			++compoundIdCounter_;
//			discreteEvalResults_.emplace_back();
//
//			assert(compoundIdCounter_ == discreteEvalResults_.size() - 1);
//		}
//
//		evalSteps_.emplace_back(EvalStep{
//			.operation = op,
//			.evalFn = std::forward<Fn>(fn),
//			.compoundId = compoundIdCounter_
//		});
//	}
//
//	int compoundIdCounter_ = -1;
//	std::vector<EvalStep> evalSteps_;
//	std::vector<bool> discreteEvalResults_;
//	std::vector<Condition> conditions_;
//};
//
//template <SomeStateEvalFn Fn>
//GirlStateEvaluator::Condition& 
//GirlStateEvaluator::Condition::And(Fn&& fn)
//{
//	using GSE = GirlStateEvaluator;
//
//	parent.evalSteps_.emplace_back(GSE::EvalStep{
//		.op = GSE::Operand::And,
//		.evalFn = std::forward<Fn>(fn)
//		.nestLevel = nestLevel
//	});
//
//	return *this;
//}
//
//template <SomeStateEvalFn Fn>
//GirlStateEvaluator::Condition& 
//GirlStateEvaluator::Condition::Or(Fn&& fn)
//{
//	using GSE = GirlStateEvaluator;
//
//	parent.evalSteps_.emplace_back(GSE::EvalStep{
//		.op = GSE::Operand::Or,
//		.evalFn = std::forward<Fn>(fn)
//		.nestLevel = nestLevel
//	});
//
//	return *this;
//}
//
//template <SomeStateEvalFn Fn>
//GirlStateEvaluator::Condition 
//GirlStateEvaluator::Condition::AndIf(Fn&& fn)
//{
//	using GSE = GirlStateEvaluator;
//
//	parent.evalSteps_.emplace_back(GSE::EvalStep{
//		.op = GSE::Operand::And,
//		.evalFn = std::forward<Fn>(fn)
//		.nestLevel = nestLevel + 1
//	});
//
//	return GSE::Condition{ parent, nestLevel + 1 };
//}
//
//template <SomeStateEvalFn Fn>
//GirlStateEvaluator::Condition
//GirlStateEvaluator::Condition::OrIf(Fn&& fn)
//{
//
//}
//
//GirlStateEvaluator::Condition
//GirlStateEvaluator::Condition::Close()
//{
//	using GSE = GirlStateEvaluator;
//
//	size_t newLvl = (nestLevel > 0) ? nestLevel - 1 : 0;
//
//	return GSE::Condition{ parent, newLvl };
//}
//
//GirlStateEvaluator::Condition
//GirlStateEvaluator::Condition::operator()()
//{
//	using GSE = GirlStateEvaluator;
//
//	size_t newLvl = (nestLevel > 0) ? nestLevel - 1 : 0;
//
//	return GSE::Condition{ parent, newLvl };
//}
//
//bool GirlStateEvaluator::ExpressionNode::Evaluate(const GirlStateContext& ctx)
//{
//	using Op = GirlStateEvaluator::Operand;
//
//	bool result = false;
//
//	if (fn)
//	{
//		result = std::invoke(fn, ctx);
//		if (op == Op::Not)
//		{
//			assert(!rhs);
//			result = !result;
//		}
//	}
//
//	if (rhs)
//	{
//		assert((op & (Op::And | Op::Or)) != 0);
//
//		bool rhsResult = rhs->Evaluate(ctx);
//		if (op & Op::Not)
//		{
//			rhsResult = !rhsResult;
//		}
//
//		if (op & Op::And)
//		{
//			result &= rhsResult;
//		}
//		else if (op & Op::Or)
//		{
//			result |= rhsResult;
//		}
//	}
//
//	return result;
//}
//
//
//} // test