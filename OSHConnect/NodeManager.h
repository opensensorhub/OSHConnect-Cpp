#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "OSHNode.h"
#include "CSAPI/Util/Utilities.h"

namespace OSHConnect {
	class NodeManager {
	private:
		std::vector<OSHNode> nodes;

	public:
		/// <summary>
		/// Add a node to OSHConnect.
		/// If a node with the same ID already exists, the node will not be added.
		/// </summary>
		/// <param name="node">The node to add.</param>
		/// <returns>The existing node if it was found, otherwise the newly added node.</returns>
		OSHNode addNode(const OSHNode& node) {
			for (const auto& existingNode : nodes) {
				if (existingNode.getUniqueId() == node.getUniqueId()) {
					return existingNode;
				}
			}

			nodes.push_back(node);
			return node;
		}

		/// <summary>
		/// Add a node to OSHConnect.
		/// If a node with the same ID already exists, the node will not be added.
		/// </summary>
		/// <param name="root">The root URL of the OpenSensorHub server, i.e., localhost:8181/sensorhub.</param>
		/// <param name="authToken">The authentication token for the node.</param>
		/// <param name="isBasicAuth">True if using Basic authentication, false if using Bearer token.</param>
		/// <param name="name">The name of the node.</param>
		/// <param name="uniqueId">The unique ID of the node.</param>
		/// <returns>The existing node if it was found, otherwise the newly added node.</returns>
		OSHNode addNode(const std::string& root, const std::string& authToken, bool isBasicAuth, const std::string& name = "", const std::string& uniqueId = "") {
			OSHNode newNode(root, authToken, isBasicAuth, name, uniqueId);
			return addNode(newNode);
		}

		/// <summary>
		/// Add a node to OSHConnect.
		/// If a node with the same ID already exists, the node will not be added.
		/// </summary>
		/// <param name="root">The root URL of the OpenSensorHub server, i.e., localhost:8181/sensorhub.</param>
		/// <param name="username">The username for Basic authentication.</param>
		/// <param name="password">The password for Basic authentication.</param>
		/// <param name="name">The name of the node.</param>
		/// <param name="uniqueId">The unique ID of the node.</param>
		/// <returns>The existing node if it was found, otherwise the newly added node.</returns>
		OSHNode addNode(const std::string& root, const std::string& username, const std::string& password, const std::string& name = "", const std::string& uniqueId = "") {
			std::string authToken = ConnectedSystemsAPI::Utilities::base64_encode(username + ":" + password);
			OSHNode newNode(root, authToken, true, name, uniqueId);
			return addNode(newNode);
		}

		/// <summary>
		/// Remove a node from OSHConnect by ID.
		/// </summary>
		/// <param name="uniqueId">The unique ID of the node to remove.</param>
		void removeNode(const std::string& uniqueId) {
			nodes.erase(std::remove_if(nodes.begin(), nodes.end(),
				[&uniqueId](const OSHNode& node) {
				return node.getUniqueId() == uniqueId;
			}), nodes.end());
		}

		/// <summary>
		/// Remove a node from OSHConnect.
		/// </summary>
		/// <param name="node">The node to remove.</param>
		void removeNode(const OSHNode& node) {
			removeNode(node.getUniqueId());
		}

		/// <summary>
		/// Remove all nodes from OSHConnect.
		/// </summary>
		void removeAllNodes() {
			nodes.clear();
		}

		/// <summary>
		/// Get a node from OSHConnect by ID.
		/// </summary>
		/// <param name="uniqueId">The unique ID of the node to get.</param>
		/// <returns>A pointer to the node if found, nullptr otherwise.</returns>
		OSHNode* getNodeById(const std::string& uniqueId) {
			for (auto& node : nodes) {
				if (node.getUniqueId() == uniqueId) {
					return &node;
				}
			}
			return nullptr;
		}

		/// <summary>
		/// Get a list of nodes in OSHConnect.
		/// </summary>
		/// <returns>A vector of all managed nodes.</returns>
		const std::vector<OSHNode>& getNodes() const {
			return nodes;
		}

		/// <summary>
		/// Shutdown the NodeManager and remove all nodes.
		/// </summary>
		void shutdown() {
			removeAllNodes();
		}
	};
}