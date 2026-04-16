#pragma once

#include <string>
#include <vector>
#include <CSAPI/DataModels/ControlStream.h>
#include <CSAPI/DataModels/Command.h>
#include <CSAPI/Query/CommandsOfControlStreamQuery.h>

namespace OSHConnect {
	class OSHSystem;

	class OSHControlStream {
	private:
		OSHSystem* parentSystem;
		ConnectedSystemsAPI::DataModels::ControlStream controlStreamResource;

		friend class OSHSystem;

	public:
		OSHControlStream(OSHSystem* parentSystem, const ConnectedSystemsAPI::DataModels::ControlStream& controlStreamResource)
			: parentSystem(parentSystem), controlStreamResource(controlStreamResource) {
		}

		/// <summary>
		/// Query the node for the latest commands of this control stream with the specified parameters.
		/// </summary>
		std::vector<ConnectedSystemsAPI::DataModels::Command> fetchCommands(std::string query = "") {
			auto response = getConSysAPI().getCommandsAPI().fetchCommandsOfControlStream(getId(), query);
			std::cout << "fetchCommands: " << response.getResponseBody() << std::endl;
			if (response.isSuccessful() && !response.getItems().empty()) {
				return response.getItems();
			}
			return {};
		}

		/// <summary>
		/// Query the node for the latest commands of this control stream with the specified parameters.
		/// </summary>
		std::vector<ConnectedSystemsAPI::DataModels::Command> fetchCommands(const ConnectedSystemsAPI::Query::CommandsOfControlStreamQuery& query) {
			return fetchCommands(query.toString());
		}

		/// <summary>
		/// Query the node for a specific command of this control stream by its ID.
		/// </summary>
		/// <param name="commandId">The ID of the command to fetch.</param>
		/// <returns>The command with the specified ID, or an empty command if not found.</returns>
		ConnectedSystemsAPI::DataModels::Command fetchCommandById(const std::string& commandId) {
			auto response = getConSysAPI().getCommandsAPI().fetchCommandById(commandId);
			if (response.isSuccessful()) {
				return response.getItem();
			}
			return ConnectedSystemsAPI::DataModels::Command{};
		}

		/// <summary>
		/// Refresh the control stream properties from the server.
		/// </summary>
		/// <returns>True if the refresh was successful, false otherwise.</returns>
		bool refreshControlStream() {
			auto response = getConSysAPI().getControlStreamsAPI().getControlStreamById(getId());
			if (response.isSuccessful()) {
				controlStreamResource = response.getItem();
				return true;
			}
			return false;
		}

		/// <summary>
		/// Create a new command for this control stream on the OpenSensorHub node using the provided command resource.
		/// </summary>
		/// <param name="command">The command resource to create.</param>
		/// <returns>The created command if successful, or std::nullopt if creation failed.</returns>
		std::string createCommand(const ConnectedSystemsAPI::DataModels::Command& command) {
			auto response = getConSysAPI().getCommandsAPI().createCommand(getId(), command);
			for (const auto& [headerKey, headerValues] : response.getHeaders()) {
				for (const auto& headerValue : headerValues) {
					if (headerKey == "Location") {
						// Extract command ID from Location header if available.
						// The Location header is in the format: /commands/{commandId}
						std::string location = headerValue;
						size_t lastSlashPos = location.find_last_of('/');
						if (lastSlashPos != std::string::npos && lastSlashPos + 1 < location.size()) {
							std::string newCommandId = location.substr(lastSlashPos + 1);
							return newCommandId;
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
		/// <param name="updatedCommand">The updated command resource.</param>
		/// <returns>True if the update was successful, false otherwise.</returns>
		bool updateCommand(const std::string& commandId, const ConnectedSystemsAPI::DataModels::Command& updatedCommand) {
			auto response = getConSysAPI().getCommandsAPI().updateCommand(commandId, updatedCommand);
			return response.isSuccessful();
		}

		/// <summary>
		/// Delete an existing command of this control stream on the OpenSensorHub node by its ID.
		/// </summary>
		/// <param name="commandId">The ID of the command to delete.</param>
		/// <returns>True if the deletion was successful, false otherwise.</returns>
		bool deleteCommand(const std::string& commandId) {
			auto response = getConSysAPI().getCommandsAPI().deleteCommand(commandId);
			return response.isSuccessful();
		}

		/// <summary>
		/// The ID of the control stream. Same as controlStreamResource.getId().
		/// </summary>
		const std::string& getId() const { return controlStreamResource.getId().value(); }

		/// <summary>
		/// The control stream resource representing this control stream.
		/// </summary>
		ConnectedSystemsAPI::DataModels::ControlStream getControlStreamResource() const { return controlStreamResource; }

		/// <summary>
		/// The ConnectedSystemsAPI instance used to communicate with the server.
		/// </summary>
		ConnectedSystemsAPI::ConSysAPI& getConSysAPI();

		OSHSystem* getParentSystem() const { return parentSystem; }
	};
}