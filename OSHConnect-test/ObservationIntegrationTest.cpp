#include <chrono>
#include <string>

#include "pch.h"
#include "CppUnitTest.h"
#include "TestHelper.h"
#include "../OSHConnect/OSHNode.h"
#include "../OSHConnect/OSHSystem.h"
#include "../OSHConnect/OSHDataStream.h"
#include "../OSHConnect/OSHConnect.h"

#include <CSAPI/DataModels/ObservationBuilder.h>
#include <CSAPI/DataModels/TimeInstant.h>
#include <CSAPI/DataModels/Component/DataRecord.h>
#include <CSAPI/DataModels/Data/DataValue.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace OSHConnectTest {
	/// <summary>
	/// Tests for OSHConnect integration with an OpenSensorHub node.
	/// Note: These tests require a running OpenSensorHub node at localhost:8282
	/// with a system and data stream available for testing.
	/// </summary>
	TEST_CLASS(ObservationIntegrationTest) {
		TestHelper helper = TestHelper();
		std::string dataStreamId;

		TEST_METHOD_INITIALIZE(ClassInitialize) {
			helper.createTestSystem();
			auto system = helper.node.getSystemById(helper.systemId).value();
			dataStreamId = helper.createTestDataStream();
		}

		TEST_METHOD_CLEANUP(MethodCleanup) {
			helper.cleanupTestSystem();
		}

		TEST_METHOD(FetchObservations) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& node = oshConnect.getNodeManager().addNode("http://localhost:8282/sensorhub", "admin", "admin");
			auto system = helper.assertNotEmptyAndGetFirst(node.discoverSystems());
			auto dataStream = helper.assertNotEmptyAndGetFirst(system.discoverDataStreams());
			auto observations = dataStream.fetchObservations();
			Assert::IsFalse(observations.empty());
		}

		TEST_METHOD(FetchObservationById) {
			OSHConnect::OSHConnect oshConnect("TestOSHConnect");
			auto& node = oshConnect.getNodeManager().addNode("http://localhost:8282/sensorhub", "admin", "admin");
			auto system = helper.assertNotEmptyAndGetFirst(node.discoverSystems());
			auto dataStream = helper.assertNotEmptyAndGetFirst(system.discoverDataStreams());
			auto observation = helper.assertNotEmptyAndGetFirst(dataStream.fetchObservations());

			if (observation.getId().has_value()) {
				auto fetchedObservation = dataStream.fetchObservationById(observation.getId().value());
				Assert::IsTrue(fetchedObservation.getId().has_value());
				Assert::AreEqual(observation.getId().value(), fetchedObservation.getId().value());
			}
		}

		TEST_METHOD(CreateObservation) {
			auto system = helper.node.getSystemById(helper.systemId).value();
			auto dataStream = system.getDataStreamById(dataStreamId).value();
			auto createdObservationId = createObservationAndGetId(dataStream);

			Assert::IsFalse(createdObservationId.empty());

			auto createdObservation = dataStream.fetchObservationById(createdObservationId);
			Assert::IsTrue(createdObservation.getId().has_value());
			Assert::AreEqual(createdObservationId, createdObservation.getId().value());
		}

		TEST_METHOD(DeleteObservation) {
			auto system = helper.node.getSystemById(helper.systemId).value();
			auto dataStream = system.getDataStreamById(dataStreamId).value();
			auto createdObservationId = createObservationAndGetId(dataStream);
			Assert::IsFalse(createdObservationId.empty());

			auto createdObservation = dataStream.fetchObservationById(createdObservationId);
			Assert::IsTrue(createdObservation.getId().has_value());

			bool deleted = dataStream.deleteObservation(createdObservationId);
			Assert::IsTrue(deleted);
		}

		std::string createObservationAndGetId(OSHConnect::OSHDataStream& dataStream) {
			//Get the schema to know what kind of observation to create
			auto dataStreamResource = dataStream.getDataStreamResource();
			auto schemaPtr = dataStreamResource.getSchema();
			Assert::IsNotNull(schemaPtr);
			auto schema = schemaPtr->getResultSchema();
			Assert::IsNotNull(schema);
			auto schemaDataRecord = dynamic_cast<const ConnectedSystemsAPI::DataModels::Component::DataRecord*>(schema);
			Assert::IsNotNull(schemaDataRecord);

			// Create a data block according to the schema
			auto dataBlock = schemaDataRecord->createDataBlock();
			dataBlock.setField("booleanField", ConnectedSystemsAPI::DataModels::Data::DataValue(true));

			auto now = ConnectedSystemsAPI::DataModels::TimeInstant(std::chrono::system_clock::now());
			auto observation = ConnectedSystemsAPI::DataModels::ObservationBuilder()
				.withResultTime(now)
				.withResult(dataBlock)
				.build();

			return dataStream.createObservation(observation);
		}
	};
}
