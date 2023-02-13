#pragma once

struct IState
{
	virtual void OnEnter(void* param) = 0;
	virtual void OnUpdate(void* param) = 0;
	virtual void OnEnd(void* param) = 0;
	//boost::any
	void* customData_{ nullptr };
};

struct StateMachine
{
	virtual void ChangeState(IState* state)
	{
		if (currentState_ != nullptr)
		{
			currentState_->OnEnd(nullptr);
		}

		previousState_ = currentState_;
		currentState_ = state;

		currentState_->OnEnd(nullptr);
	}
	virtual void ChangeToPrevious()
	{
		auto tempState = currentState_;
		currentState_ = previousState_;
		previousState_ = tempState;
	}
	virtual void Update()
	{
		if (globalState_ != nullptr)
		{
			globalState_->OnUpdate(nullptr);
		}

		if (currentState_ != nullptr)
		{
			currentState_->OnUpdate(nullptr);
		}
	}

	IState* currentState_{ nullptr };
	IState* previousState_{ nullptr };
	IState* globalState_{ nullptr };
};