#ifndef GATEWAY_FAKEHANDLER_H
#define GATEWAY_FAKEHANDLER_H

#include "core/CommandHandler.h"

namespace BeeeOn {

class FakeHandler : public CommandHandler, public Loggable {
public:
	FakeHandler():
		CommandHandler("FakeHandler")
	{
		logger().warning("Run");
	}

	bool accept(const Command::Ptr cmd);
	void handle(Command::Ptr cmd, Answer::Ptr answer);
};

}

#endif
