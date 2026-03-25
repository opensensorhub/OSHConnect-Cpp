#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include <CSAPI/ConnectedSystemsAPI.h>
#include <CSAPI/DataModels/System.h>
#include <CSAPI/Query/SystemsQuery.h>

#include "OSHSystem.h"
#include "Constants/Service.h"
#include "Util/Utilities.h"

namespace OSHConnect {
	/// <summary>
	/// Class representing an OpenSensorHub server instance or node.
	/// </summary>
	class OSHNode {
	private:
		std::string sensorHubRoot;
		std::string authenticationToken;
		bool isSecure{ false };
		bool isBasicAuth{ false };
		ConnectedSystemsAPI::ConSysAPI conSysAPI;
		std::string name{ "OSH Node" };
		std::string uniqueId;
		std::vector<OSHSystem> systems{};

		/// <summary>
		/// Adds a new system or updates an existing one based on the provided system resource.
		/// </summary>
		OSHSystem addOrUpdateSystem(const ConnectedSystemsAPI::DataModels::System& systemResource) {
			// Update the existing system if there is one
			for (auto& sys : systems) {
				if (sys.getId() == systemResource.getId()) {
					sys.systemResource = systemResource;
					return sys;
				}
			}

			// Add a new system if not found
			OSHSystem newSystem(this, systemResource);
			systems.push_back(newSystem);
			return newSystem;
		}

		/// <summary>
		/// Generates a unique ID for the node based on its root URL.
		/// This should be good enough for most cases.
		/// </summary>
		/// <returns>A unique ID string based on the root URL.</returns>
		static std::string generateIdFromRoot(const std::string& root) {
			std::hash<std::string> hasher;
			return "node-" + std::to_string(hasher(root));
		}

	public:
		explicit OSHNode(const std::string& root,
			const std::string& authToken,
			bool isBasicAuth,
			const std::string& name = "",
			const std::string& uniqueId = "")
			: sensorHubRoot(Util::Utilities::stripProtocol(root)),
			authenticationToken(authToken),
			isSecure(root.find("https://") == 0),
			isBasicAuth(isBasicAuth),
			conSysAPI(ConnectedSystemsAPI::ConSysAPI(getApiEndpoint(), authToken, isBasicAuth)),
			name(name.empty() ? "OSH Node" : name),
			uniqueId(uniqueId.empty() ? generateIdFromRoot(sensorHubRoot) : uniqueId) {
		}

		/// <summary>
		/// Query the OpenSensorHub node for systems.
		/// New systems will be added to the list of discovered systems,
		/// and existing systems will have their resources updated.
		/// </summary>
		/// <returns>The list of discovered systems.</returns>
		std::vector<OSHSystem> discoverSystems(std::string query = "") {
			auto systemsResponse = conSysAPI.getSystemsAPI().getSystems(query);
			std::vector<OSHSystem> discoveredSystems;

			if (systemsResponse.isSuccessful()) {
				for (const auto& systemResource : systemsResponse.getItems()) {
					auto newSystem = addOrUpdateSystem(systemResource);
					discoveredSystems.push_back(newSystem);
				}
			}

			return discoveredSystems;
		}

		/// <summary>
		/// Query the OpenSensorHub node for systems.
		/// New systems will be added to the list of discovered systems,
		/// and existing systems will have their resources updated.
		/// </summary>
		/// <returns>The list of discovered systems.</returns>
		std::vector<OSHSystem> discoverSystems(const ConnectedSystemsAPI::Query::SystemsQuery& query) {
			return discoverSystems(query.toString());
		}

		/// <summary>
		/// Create a new system on the OpenSensorHub node using the provided system resource.
		/// </summary>
		/// <param name="systemResource">The system resource to create.</param>
		/// <returns>The ID of the newly created system, or std::nullopt if creation failed.</returns>
		std::optional<OSHSystem> createSystem(const ConnectedSystemsAPI::DataModels::System& systemResource) {
			auto response = conSysAPI.getSystemsAPI().createSystem(systemResource);

			for (const auto& [headerKey, headerValues] : response.getHeaders()) {
				for (const auto& headerValue : headerValues) {
					if (headerKey == "Location") {
						// Extract system ID from Location header if available.
						// The Location header is in the format: /systems/{systemId}
						std::string location = headerValue;
						size_t lastSlashPos = location.find_last_of('/');
						if (lastSlashPos != std::string::npos && lastSlashPos + 1 < location.size()) {
							std::string newSystemId = location.substr(lastSlashPos + 1);
							return getSystemById(newSystemId);
						}
					}
				}
			}
			return std::nullopt;
		}

		/// <summary>
		/// Delete a system from the OpenSensorHub node and remove it from the list of discovered systems.
		/// </summary>
		/// <param name="systemId">The ID of the system to delete.</param>
		/// <param name="cascade">If true, all associated sub-resources hosted by the same server,
		///                       (sampling features, data streams, command streams, observations, and commands)
		///                       are also deleted.</param>
		/// <returns>True if the system was successfully deleted, false otherwise.</returns>
		bool deleteSystem(const std::string& systemId, const bool cascade = false) {
			auto response = conSysAPI.getSystemsAPI().deleteSystem(systemId, cascade);
			if (response.isSuccessful()) {
				systems.erase(std::remove_if(systems.begin(), systems.end(),
					[&systemId](const OSHSystem& sys) { return sys.getId() == systemId; }),
					systems.end());
				return true;
			}
			return false;
		}

		/// <summary>
		/// Get a system by its ID and add it to the list of discovered systems if not already present.
		/// If the system is already in the list of discovered systems,
		/// it will be updated with the latest resource from the server.
		/// </summary>
		/// <param name="systemId">The ID of the system to retrieve.</param>
		/// <returns>An optional containing the system if found, or std::nullopt if not found.</returns>
		std::optional<OSHSystem> getSystemById(const std::string& systemId) {
			auto response = conSysAPI.getSystemsAPI().getSystemById(systemId);
			if (response.isSuccessful()) {
				return addOrUpdateSystem(response.getItem());
			}
			return std::nullopt;
		}

		/// <summary>
		/// The root URL of the OpenSensorHub server, i.e., localhost:8181/sensorhub.
		/// </summary>
		const std::string& getSensorHubRoot() const { return sensorHubRoot; }
		/// <summary>
		/// Flag indicating if server is secured through TLS/SSL.
		/// </summary>
		bool getIsSecure() const { return isSecure; }
		/// <summary>
		/// Flag indicating if Basic authentication is used (true) or Bearer token authentication (false).
		/// </summary>
		bool getIsBasicAuth() const { return isBasicAuth; }
		/// <summary>
		/// Friendly name for the server.
		/// </summary>
		const std::string& getName() const { return name; }
		void setName(const std::string& newName) { name = newName; }
		/// <summary>
		/// A unique id of the server, for configuration management.
		/// </summary>
		const std::string& getUniqueId() const { return uniqueId; }
		/// <summary>
		/// The authentication token used to access the server.
		/// </summary>
		const std::string& getAuthenticationToken() const { return authenticationToken; }

		/// <summary>
		/// The ConnectedSystemsAPI instance used to communicate with the server.
		/// </summary>
		ConnectedSystemsAPI::ConSysAPI& getConSysAPI() { return conSysAPI; }

		std::string getHTTPPrefix() const { return isSecure ? "https://" : "http://"; }
		std::string getWSPrefix() const { return isSecure ? "wss://" : "ws://"; }
		std::string getApiEndpoint() const { return Util::Utilities::joinPath(sensorHubRoot, Constants::Service::API); }
		std::string getSystemsEndpoint() const { return Util::Utilities::joinPath(sensorHubRoot, Constants::Service::SYSTEMS); }
	};

	inline ConnectedSystemsAPI::ConSysAPI& OSHSystem::getConSysAPI() {
		return parentNode->getConSysAPI();
	}
}