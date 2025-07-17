#pragma once

class INotifiable
{
public:
	virtual ~INotifiable() = default;
	virtual void OnClick() = 0;
};