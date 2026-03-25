#include "pch.h"
#include "CppUnitTest.h"

#include <exception>
#include <iostream>
#include <string>

#include "../OSHConnect/OSHConnect.h"
#include "../OSHConnect/OSHNode.h"
#include "../OSHConnect/OSHSystem.h"
#include "../OSHConnect/OSHDataStream.h"
#include "../OSHConnect/Websocket/StreamListener.h"
#include "../OSHConnect/Websocket/StreamHandler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace OSHConnectTest
{
	/// <summary>
	/// Tests for OSHConnect integration with an OpenSensorHub node.
	/// Note: These tests require a running OpenSensorHub node at localhost:8282
	/// with a system and data stream available for testing.
	/// </summary>
	TEST_CLASS(WebsocketIntegrationTest)
	{
		TEST_METHOD(OSHWebsocketTest) {
			auto oshConnect = OSHConnect::OSHConnect("TestOSHConnect");
			auto node = oshConnect.getNodeManager().addNode("http://localhost:8282/sensorhub", "admin", "admin");
			auto systems = node.discoverSystems();
			std::string systemId;
			for (const auto& system : systems) {
				std::cout << "Discovered system: " + system.getSystemResource().getProperties().getName() << std::endl;
				systemId = system.getId();
			}

			auto system = node.getSystemById(systemId);
			auto dataStreams = system->discoverDataStreams();
			std::string dataStreamId;
			for (const auto& ds : dataStreams) {
				std::cout << "Discovered data stream: " + ds.getDataStreamResource().getName().value_or("") << std::endl;
				dataStreamId = ds.getId();
			}
			auto dataStream = system->getDataStreamById(dataStreamId);

			auto& streamManager = oshConnect.getStreamManager();

			// Track message count and listener outside the callback
			int messageCount = 0;
			const int maxMessages = 5;
			OSHConnect::Websocket::StreamListener* activeListener = nullptr;

			// Create handler using StreamManager
			auto* handler = streamManager.createDataStreamHandler(
				[&messageCount, &maxMessages, &activeListener](const OSHConnect::Websocket::StreamEventArgs& args) {
				// Display received data with metadata
				std::cout << "Received observation #" << (messageCount + 1) << std::endl;
				std::cout << "  Stream ID: " << args.getDataStream()->getId() << std::endl;
				std::cout << "  Timestamp: " << args.getTimestamp() << " ms" << std::endl;
				std::cout << "  Format: " << static_cast<int>(args.getFormat()) << std::endl;

				if (args.getObservation().has_value()) {
					std::cout << "  Data: " << std::endl << args.getObservation().value() << std::endl;
				}

				messageCount++;

				if (messageCount >= maxMessages && activeListener) {
					activeListener->disconnect();
				}
			});

			try {
				// Add data stream to handler
				activeListener = handler->addDataStreamListener(*dataStream);

				// Connect all streams in the handler
				handler->connect();
				std::cout << "Connected to OSH WebSocket for data stream " + dataStreamId << std::endl;

				// Listen will block until we receive maxMessages and disconnect
				// Note: In a real application with multiple streams, each listener would run in its own thread
				activeListener->listen();

				Assert::AreEqual(5, messageCount, L"Should receive 5 messages");
				std::cout << "Successfully received " + std::to_string(messageCount) + " messages" << std::endl;

				// Clean shutdown - StreamManager will handle cleanup
				streamManager.shutdownDataStreamHandler(handler);
			}
			catch (const std::exception& ex) {
				Logger::WriteMessage(("Error: " + std::string(ex.what())).c_str());
				Assert::Fail(L"WebSocket listener test failed");
			}
		}
	};
}