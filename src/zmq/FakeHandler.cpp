#include <Poco/Logger.h>
#include <commands/ServerLastValueResult.h>

#include "di/Injectable.h"
#include "zmq/FakeHandler.h"

BEEEON_OBJECT_BEGIN(BeeeOn, FakeHandler)
BEEEON_OBJECT_CASTABLE(CommandHandler)
BEEEON_OBJECT_END(BeeeOn, FakeHandler)

using namespace BeeeOn;

bool FakeHandler::accept(const Command::Ptr)
{
	logger().warning("accept FakeHandler1");
	return true;
}

void FakeHandler::handle(Command::Ptr, Answer::Ptr answer)
{
	Poco::Thread::sleep(500);
	logger().warning("handle FakeHandler1");

	ServerLastValueResult::Ptr res = new ServerLastValueResult(answer);
	res->setValue(0);
	res->setStatus(Result::SUCCESS);


}
