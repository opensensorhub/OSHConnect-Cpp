#include <string>

#include "pch.h"
#include "CppUnitTest.h"
#include "../OSHConnect/OSHConnect.h"
#include "../OSHConnect/OSHNode.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace OSHConnectTest
{
	TEST_CLASS(NodeManagerTest)
	{
	public:
		TEST_METHOD(AddNode_ByCredentials) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& nodeManager = oshConnect.getNodeManager();
			OSHConnect::OSHNode addedNode = nodeManager.addNode("localhost:8282/sensorhub", "admin", "admin", "TestNode", "node-001");
			Assert::AreEqual(std::string("node-001"), addedNode.getUniqueId());
		}

		TEST_METHOD(AddNode_ByNode) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& nodeManager = oshConnect.getNodeManager();
			OSHConnect::OSHNode newNode("localhost:8282/sensorhub", "admin", "admin", "TestNode", "node-001");
			OSHConnect::OSHNode addedNode = nodeManager.addNode(newNode);
			Assert::AreEqual(std::string("node-001"), addedNode.getUniqueId());
		}

		TEST_METHOD(AddNode_DuplicateNode) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& nodeManager = oshConnect.getNodeManager();
			OSHConnect::OSHNode newNode("localhost:8282/sensorhub", "admin", "admin", "TestNode", "node-001");
			nodeManager.addNode(newNode);
			OSHConnect::OSHNode addedNode = nodeManager.addNode(newNode);
			Assert::AreEqual(std::string("node-001"), addedNode.getUniqueId());
		}

		TEST_METHOD(GetNodes_Count) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& nodeManager = oshConnect.getNodeManager();
			OSHConnect::OSHNode newNode1("localhost:8282/sensorhub", "admin", "admin", "TestNode1", "node-001");
			OSHConnect::OSHNode newNode2("localhost:8282/sensorhub", "admin", "admin", "TestNode2", "node-002");
			nodeManager.addNode(newNode1);
			nodeManager.addNode(newNode2);
			const auto& nodes = nodeManager.getNodes();
			Assert::AreEqual(size_t(2), nodes.size());
		}

		TEST_METHOD(RemoveNode_ById) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& nodeManager = oshConnect.getNodeManager();
			nodeManager.addNode("localhost:8282/sensorhub", "admin", "admin", "TestNode", "node-001");
			nodeManager.removeNode("node-001");
			const auto& nodes = nodeManager.getNodes();
			Assert::AreEqual(size_t(0), nodes.size());
		}

		TEST_METHOD(RemoveNode_ByInvalidId) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& nodeManager = oshConnect.getNodeManager();
			nodeManager.addNode("localhost:8282/sensorhub", "admin", "admin", "TestNode", "node-001");
			nodeManager.removeNode("invalid-id");
			const auto& nodes = nodeManager.getNodes();
			Assert::AreEqual(size_t(1), nodes.size());
		}

		TEST_METHOD(RemoveNode_ByNode) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& nodeManager = oshConnect.getNodeManager();
			OSHConnect::OSHNode newNode("localhost:8282/sensorhub", "admin", "admin", "TestNode", "node-001");
			nodeManager.addNode(newNode);
			nodeManager.removeNode(newNode);
			const auto& nodes = nodeManager.getNodes();
			Assert::AreEqual(size_t(0), nodes.size());
		}
	};
}
