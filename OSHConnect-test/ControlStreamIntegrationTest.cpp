#include <string>

#include "pch.h"
#include "CppUnitTest.h"
#include "TestHelper.h"
#include "../OSHConnect/OSHNode.h"
#include "../OSHConnect/OSHSystem.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace OSHConnectTest {
	/// <summary>
	/// Tests for OSHConnect integration with an OpenSensorHub node.
	/// Note: These tests require a running OpenSensorHub node at localhost:8282
	/// with a system and control stream available for testing.
	/// </summary>
	TEST_CLASS(ControlStreamIntegrationTest) {
		TestHelper helper = TestHelper();

		TEST_METHOD(TestControlStreams) {
			auto systems = helper.node.discoverSystems();
			auto system = TestHelper::assertNotEmptyAndGetFirst(systems);
			auto controlStreams = system.discoverControlStreams();
			Assert::IsFalse(controlStreams.empty());
			auto& controlStream = TestHelper::assertNotEmptyAndGetFirst(controlStreams);
			std::cout << "Control Stream: " << controlStream.getId() << std::endl;

			auto commands = controlStream.fetchCommands();
			for (const auto& command : commands) {
				std::cout << "Command: " << command.toJson().dump(2) << std::endl;
			}
		}
	};
}