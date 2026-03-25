#pragma once

#include <string>

#include "NodeManager.h"
#include "StreamManager.h"

namespace OSHConnect {
	class OSHConnect {
	private:
		std::string name;
		NodeManager nodeManager = NodeManager();
		StreamManager streamManager = StreamManager();

	public:
		explicit OSHConnect(const std::string& name = "OSHConnect") : name(name) {}

		/// <summary>
		/// The name of the OSHConnect instance.
		/// </summary>
		std::string getName() const { return name; }

		/// <summary>
		/// Get the NodeManager instance.
		/// </summary>
		NodeManager& getNodeManager() { return nodeManager; }

		/// <summary>
		/// Get the StreamManager instance.
		/// </summary>
		StreamManager& getStreamManager() { return streamManager; }
	};
}