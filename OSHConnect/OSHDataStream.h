#pragma once

#include <string>
#include <vector>
#include <CSAPI/DataModels/DataStream.h>
#include <CSAPI/DataModels/Observation.h>
#include <CSAPI/ConnectedSystemsAPI.h>
#include <CSAPI/Query/ObservationsOfDataStreamQuery.h>

#include "Constants/Service.h"

namespace OSHConnect {
	class OSHSystem;

	class OSHDataStream {
	private:
		OSHSystem* parentSystem;
		ConnectedSystemsAPI::DataModels::DataStream dataStreamResource;

		friend class OSHSystem;

	public:
		OSHDataStream(OSHSystem* parentSystem, const ConnectedSystemsAPI::DataModels::DataStream& dataStreamResource)
			: parentSystem(parentSystem), dataStreamResource(dataStreamResource) {
		}

		/// <summary>
		/// Query the node for the latest observations of this data stream with the specified parameters.
		/// </summary>
		std::vector<ConnectedSystemsAPI::DataModels::Observation> fetchObservations(std::string query = "") {
			auto response = getConSysAPI().getObservationsAPI().getObservationsOfDataStream(getId(), query);
			if (response.isSuccessful() && !response.getItems().empty()) {
				return response.getItems();
			}
			return {};
		}

		/// <summary>
		/// Query the node for the latest observations of this data stream with the specified parameters.
		/// </summary>
		std::vector<ConnectedSystemsAPI::DataModels::Observation> fetchObservations(const ConnectedSystemsAPI::Query::ObservationsOfDataStreamQuery& query) {
			return fetchObservations(query.toString());
		}

		/// <summary>
		/// Query the node for a specific observation of this data stream by its ID.
		/// </summary>
		/// <param name="observationId">The ID of the observation to fetch.</param>
		/// <returns>The observation with the specified ID, or an empty observation if not found.</returns>
		ConnectedSystemsAPI::DataModels::Observation fetchObservationById(const std::string& observationId) {
			auto response = getConSysAPI().getObservationsAPI().getObservationById(observationId);
			if (response.isSuccessful()) {
				return response.getItem();
			}
			return ConnectedSystemsAPI::DataModels::Observation{};
		}

		/// <summary>
		/// Refresh the data stream properties from the server.
		/// </summary>
		/// <returns>True if the refresh was successful, false otherwise.</returns>
		bool refreshDataStream() {
			auto response = getConSysAPI().getDataStreamsAPI().getDataStreamById(getId());
			if (response.isSuccessful()) {
				dataStreamResource = response.getItem();
				return true;
			}
			return false;
		}

		/// <summary>
		/// Create a new observation for this data stream on the OpenSensorHub node using the provided observation resource.
		/// </summary>
		/// <param name="observation">The observation resource to create.</param>
		/// <returns>The created observation if successful, or std::nullopt if creation failed.</returns>
		std::string createObservation(const ConnectedSystemsAPI::DataModels::Observation& observation) {
			auto response = getConSysAPI().getObservationsAPI().createObservation(getId(), observation);
			for (const auto& [headerKey, headerValues] : response.getHeaders()) {
				for (const auto& headerValue : headerValues) {
					if (headerKey == "Location") {
						// Extract observation ID from Location header if available.
						// The Location header is in the format: /observations/{observationId}
						std::string location = headerValue;
						size_t lastSlashPos = location.find_last_of('/');
						if (lastSlashPos != std::string::npos && lastSlashPos + 1 < location.size()) {
							std::string newObservationId = location.substr(lastSlashPos + 1);
							return newObservationId;
						}
					}
				}
			}
			return "";
		}

		/// <summary>
		/// Update an existing observation of this data stream on the OpenSensorHub node using the provided observation resource.
		/// </summary>
		/// <param name="observationId">The ID of the observation to update.</param>
		/// <param name="updatedObservation">The updated observation resource.</param>
		/// <returns>True if the update was successful, false otherwise.</returns>
		bool updateObservation(const std::string& observationId, const ConnectedSystemsAPI::DataModels::Observation& updatedObservation) {
			auto response = getConSysAPI().getObservationsAPI().updateObservation(observationId, updatedObservation);
			return response.isSuccessful();
		}

		/// <summary>
		/// Delete an existing observation of this data stream on the OpenSensorHub node by its ID.
		/// </summary>
		/// <param name="observationId">The ID of the observation to delete.</param>
		/// <returns>True if the deletion was successful, false otherwise.</returns>
		bool deleteObservation(const std::string& observationId) {
			auto response = getConSysAPI().getObservationsAPI().deleteObservation(observationId);
			return response.isSuccessful();
		}

		/// <summary>
		/// The ID of the data stream. Same as dataStreamResource.getId().
		/// </summary>
		const std::string& getId() const { return dataStreamResource.getId().value(); }

		/// <summary>
		/// The data stream resource representing this data stream.
		/// </summary>
		ConnectedSystemsAPI::DataModels::DataStream getDataStreamResource() const { return dataStreamResource; }

		/// <summary>
		/// The ConnectedSystemsAPI instance used to communicate with the server.
		/// </summary>
		ConnectedSystemsAPI::ConSysAPI& getConSysAPI();

		OSHSystem* getParentSystem() const { return parentSystem; }
	};
}