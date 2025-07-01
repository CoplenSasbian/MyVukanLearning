
#pragma once
#include <vkd/context.h>
class AppContext :public vkd::ImMoveable, vkd::NonCopyable {
public:
	static AppContext& Instance() {
		static AppContext instance;
		return instance;
	}

	void run();
private:
	AppContext();

	vkd::exec::Context execContext_;

};