#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <CSAPI/DataModels/System.h>
#include <CSAPI/Query/DataStreamsOfSystemQuery.h>
#include <CSAPI/ConnectedSystemsAPI.h>
#include <CSAPI/DataModels/DataStreamBuilder.h>
#include <CSAPI/DataModels/DataStream.h>

#include "OSHDataStream.h"

namespace OSHConnect {
	class OSHNode;

	class OSHSystem {
	private:
		OSHNode* parentNode;
		ConnectedSystemsAPI::DataModels::System systemResource;
		std::vector<OSHDataStream> discoveredDataStreams{};

		OSHDataStream addOrUpdateDataStream(const ConnectedSystemsAPI::DataModels::DataStream& dataStreamResource) {
			if (!dataStreamResource.getId().has_value()) {
				throw std::invalid_argument("Data stream resource must have an ID to be added or updated.");
			}

			auto response = getConSysAPI().getDataStreamsAPI().getObservationSchema(dataStreamResource.getId().value());

			// Create a new builder with the data stream resource and the fetched observation schema
			ConnectedSystemsAPI::DataModels::DataStreamBuilder builder(dataStreamResource);
			builder.withSchema(response.getItem());
			auto dataStreamResourceNew = builder.build();

			// Update the existing data stream if there is one
			for (auto& ds : discoveredDataStreams) {
				if (ds.getId() == dataStreamResourceNew.getId()) {
					ds.dataStreamResource = dataStreamResourceNew;
					return ds;
				}
			}

			// Add a new data stream if not found
			OSHDataStream newDataStream(this, dataStreamResourceNew);
			discoveredDataStreams.push_back(newDataStream);
			return newDataStream;
		}

		friend class OSHNode;

	public:
		OSHSystem(OSHNode* parentNode, const ConnectedSystemsAPI::DataModels::System& systemResource)
			: parentNode(parentNode), systemResource(systemResource) {
		}

		/// <summary>
		/// Query the node for the data streams associated with the system.
		/// New data streams will be added to the list of discovered data streams,
		/// and existing data streams will have their resources updated.
		/// </summary>
		/// <param name="query">Optional query string to filter data streams.</param>
		/// <returns>The list of discovered data streams.</returns>
		std::vector<OSHDataStream> discoverDataStreams(std::string query = "") {
			auto response = getConSysAPI().getDataStreamsAPI().getDataStreamsOfSystem(getId(), query);
			std::vector<OSHDataStream> discoveredStreams;
			if (response.isSuccessful()) {
				for (const auto& dataStreamResource : response.getItems()) {
					auto newDataStream = addOrUpdateDataStream(dataStreamResource);
					discoveredStreams.push_back(newDataStream);
				}
			}
			return discoveredStreams;
		}

		/// <summary>
		/// Query the node for the data streams associated with the system.
		/// New data streams will be added to the list of discovered data streams,
		/// and existing data streams will have their resources updated.
		/// </summary>
		/// <param name="query">Optional query string to filter data streams.</param>
		/// <returns>The list of discovered data streams.</returns>
		std::vector<OSHDataStream> discoverDataStreams(const ConnectedSystemsAPI::Query::DataStreamsOfSystemQuery& query) {
			return discoverDataStreams(query.toString());
		}

		/// <summary>
		/// Refresh the system properties from the server.
		/// </summary>
		/// <returns>True if the refresh was successful, false otherwise.</returns>
		bool refreshSystem() {
			auto response = getConSysAPI().getSystemsAPI().getSystemById(getId());
			if (response.isSuccessful()) {
				systemResource = response.getItem();
				return true;
			}
			return false;
		}

		/// <summary>
		/// Create a new data stream on the OpenSensorHub node using the provided data stream resource.
		/// </summary>
		/// <param name="dataStreamResource">The data stream resource to create.</param>
		/// <returns>The ID of the newly created data stream, or std::nullopt if creation failed.</returns>
		std::optional<OSHDataStream> createDataStream(const ConnectedSystemsAPI::DataModels::DataStream& dataStreamResource) {
			auto response = getConSysAPI().getDataStreamsAPI().createDataStream(getId(), dataStreamResource);

			for (const auto& [headerKey, headerValues] : response.getHeaders()) {
				for (const auto& headerValue : headerValues) {
					if (headerKey == "Location") {
						// Extract system ID from Location header if available.
						// The Location header is in the format: /datastreams/{dataStreamId}
						std::string location = headerValue;
						size_t lastSlashPos = location.find_last_of('/');
						if (lastSlashPos != std::string::npos && lastSlashPos + 1 < location.size()) {
							std::string newDataStreamId = location.substr(lastSlashPos + 1);
							return getDataStreamById(newDataStreamId);
						}
					}
				}
			}
			return std::nullopt;
		}

		/// <summary>
		/// Delete a data stream from the OpenSensorHub node and remove it from the list of discovered data streams.
		/// </summary>
		/// <param name="dataStreamId">The ID of the data stream to delete.</param>
		/// <param name="cascade">If true, all associated sub-resources hosted by the same server,
		///                       (sampling features, data streams, command streams, observations, and commands)
		///                       are also deleted.</param>
		/// <returns>True if the data stream was successfully deleted, false otherwise.</returns>
		bool deleteDataStream(const std::string& dataStreamId, const bool cascade = false) {
			auto response = getConSysAPI().getDataStreamsAPI().deleteDataStream(dataStreamId, cascade);
			if (response.isSuccessful()) {
				discoveredDataStreams.erase(std::remove_if(discoveredDataStreams.begin(), discoveredDataStreams.end(),
					[&dataStreamId](const OSHDataStream& ds) { return ds.getId() == dataStreamId; }),
					discoveredDataStreams.end());
				return true;
			}
			return false;
		}

		/// <summary>
		/// Update the system properties on the server.
		/// Note: After updating the system, the system properties are refreshed from the server,
		/// not set to the provided system properties.
		/// </summary>
		/// <returns>True if the update was successful, false otherwise.</returns>
		bool updateSystem(const ConnectedSystemsAPI::DataModels::System& updatedSystemResource) {
			if (auto response = getConSysAPI().getSystemsAPI().updateSystem(getId(), updatedSystemResource); response.isSuccessful()) {
				return refreshSystem();
			}
			return false;
		}

		/// <summary>
		/// Get a data stream by its ID and add it to the list of discovered data streams if not already present.
		/// If the data stream is already in the list of discovered data streams,
		/// it will be updated with the latest resource from the server.
		/// </summary>
		/// <param name="dataStreamId">The ID of the data stream to retrieve.</param>
		/// <returns>An optional containing the data stream if found, or std::nullopt if not found.</returns>
		std::optional<OSHDataStream> getDataStreamById(const std::string& dataStreamId) {
			auto response = getConSysAPI().getDataStreamsAPI().getDataStreamById(dataStreamId);
			if (response.isSuccessful()) {
				return addOrUpdateDataStream(response.getItem());
			}
			return std::nullopt;
		}

		/// <summary>
		/// The system resource representing this system.
		/// </summary>
		ConnectedSystemsAPI::DataModels::System getSystemResource() const { return systemResource; }
		/// <summary>
		/// The ID of the system. Same as systemResource.getId().
		/// </summary>
		std::string getId() const { return systemResource.getId(); }
		/// <summary>
		/// The ConnectedSystemsAPI instance used to communicate with the server.
		/// </summary>
		ConnectedSystemsAPI::ConSysAPI& getConSysAPI();

		OSHNode* getParentNode() const { return parentNode; }
	};

	inline ConnectedSystemsAPI::ConSysAPI& OSHDataStream::getConSysAPI() {
		return parentSystem->getConSysAPI();
	}
}