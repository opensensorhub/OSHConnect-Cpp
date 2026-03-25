#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <ostream>
#include <utility>
#include <memory>
#include <CppUnitTestAssert.h>

#include <CSAPI/DataModels/SystemBuilder.h>
#include <CSAPI/DataModels/PropertiesBuilder.h>
#include <CSAPI/DataModels/DataStreamBuilder.h>
#include <CSAPI/DataModels/ObservationSchemaBuilder.h>
#include <CSAPI/DataModels/Component/DataRecordBuilder.h>
#include <CSAPI/DataModels/Component/BooleanBuilder.h>
#include <CSAPI/DataModels/Component/Boolean.h>
#include <CSAPI/DataModels/Component/DataRecord.h>
#include <CSAPI/DataModels/ObservationSchema.h>
#include <CSAPI/DataModels/DataStream.h>

#include "../OSHConnect/OSHConnect.h"
#include "../OSHConnect/OSHNode.h"

namespace OSHConnectTest {
	class TestHelper {
	public:
		static constexpr std::string_view TEST_SYSTEM_UID = "test-sensor-001";

		OSHConnect::OSHConnect oshConnect{ "TestOSHConnect" };
		OSHConnect::OSHNode node{ oshConnect.getNodeManager().addNode("http://localhost:8282/sensorhub", "admin", "admin") };
		std::string systemId;

		template<typename T>
		static auto& assertNotEmptyAndGetFirst(T& collection) {
			Assert::IsFalse(collection.empty());
			return collection.front();
		}

		std::string createTestSystem() {
			auto systemResource = createTestSystemObject();
			systemId = node.createSystem(systemResource).value().getId();
			return systemId;
		}

		ConnectedSystemsAPI::DataModels::System createTestSystemObject() {
			return ConnectedSystemsAPI::DataModels::SystemBuilder()
				.withType("Feature")
				.withProperties(ConnectedSystemsAPI::DataModels::PropertiesBuilder()
					.withFeatureType("http://www.w3.org/ns/sosa/Sensor")
					.withUid(TEST_SYSTEM_UID)
					.withName("Test Sensor 001")
					.withDescription("Test sensor created by OSHConnect-CPP integration test")
					.withAssetType("Equipment")
					.build())
				.build();
		}

		void cleanupTestSystem() {
			cleanupTestSystem(systemId);
		}

		void cleanupTestSystem(std::string const& id) {
			if (!id.empty()) {
				node.deleteSystem(id, true);
			}

			// Verify deletion
			auto response = node.getSystemById(id);
			if (response.has_value()) {
				std::cout << "Failed to clean up test system." << std::endl;
			}
		}

		std::string createTestDataStream() {
			auto dataStreamResource = createTestDataStreamObject();
			auto system = node.getSystemById(systemId).value();
			auto dataStreamOpt = system.createDataStream(dataStreamResource);
			return dataStreamOpt.value().getId();
		}

		ConnectedSystemsAPI::DataModels::DataStream createTestDataStreamObject() {
			using namespace ConnectedSystemsAPI::DataModels;
			return DataStreamBuilder()
				.withName("Test DataStream 001")
				.withOutputName("test_output_001")
				.withDescription("This is a test data stream created by CSAPI-test")
				.withSchema(std::make_unique<ObservationSchema>(ObservationSchemaBuilder()
					.withObservationFormat("application/om+json")
					.withResultSchema(std::make_unique<Component::DataRecord>(Component::DataRecordBuilder()
						.withType("DataRecord")
						.addField(std::make_unique<Component::Boolean>(Component::BooleanBuilder()
							.withType("Boolean")
							.withName("booleanField")
							.withDescription("This is a test boolean field")
							.build()))
						.build()))
					.build()))
				.build();
		}
	};
}