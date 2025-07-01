#include "init/init.h"
#include "init/context.h"



auto main()->int{

	auto& app = AppContext::Instance();

	app.run();

	return 0;

}