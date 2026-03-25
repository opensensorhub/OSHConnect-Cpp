#include <optional>

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
	/// with a system and data stream available for testing.
	/// </summary>
	TEST_CLASS(SystemIntegrationTest) {
		TestHelper helper = TestHelper();

		TEST_METHOD_INITIALIZE(ClassInitialize) {
			helper.createTestSystem();
		}

		TEST_METHOD_CLEANUP(MethodCleanup) {
			helper.cleanupTestSystem();
		}

		TEST_METHOD(CreateSystem) {
			Assert::IsFalse(helper.systemId.empty());
		}

		TEST_METHOD(DiscoverSystems) {
			auto systems = helper.node.discoverSystems();
			Assert::IsFalse(systems.empty());
		}

		TEST_METHOD(GetSystemById) {
			auto systemOpt = helper.node.getSystemById(helper.systemId);
			Assert::IsTrue(systemOpt.has_value());
		}

		TEST_METHOD(RefreshSystem) {
			auto system = TestHelper::assertNotEmptyAndGetFirst(helper.node.discoverSystems());
			bool refreshed = system.refreshSystem();
			Assert::IsTrue(refreshed);
		}

		TEST_METHOD(DeleteSystem) {
			Assert::IsFalse(helper.systemId.empty());
			bool deleted = helper.node.deleteSystem(helper.systemId, true);
			Assert::IsTrue(deleted);
		}
	};
}
