#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <string>
#include <nlohmann/json_fwd.hpp>

#include "../OSHDataStream.h"
#include "../Constants/RequestFormat.h"

namespace OSHConnect::Websocket {

	/// <summary>
	/// Event arguments for data stream events.
	/// Contains the raw data received from a WebSocket stream along with metadata.
	/// </summary>
	class StreamEventArgs {
	private:
		int64_t timestamp;
		std::vector<uint8_t> data;
		Constants::RequestFormat format;
		OSHDataStream* dataStream;

	public:
		StreamEventArgs(int64_t timestamp, const std::vector<uint8_t>& data,
			Constants::RequestFormat format, OSHDataStream* dataStream)
			: timestamp(timestamp), data(data), format(format), dataStream(dataStream) {
		}

		/// <summary>
		/// Returns the data as an Observation object or nullopt if the data is not in JSON format.
		/// </summary>
		/// <returns>An Observation object, or nullopt if parsing fails.</returns>
		std::optional<ConnectedSystemsAPI::DataModels::Observation> getObservation() const {
			if (format != Constants::RequestFormat::JSON) return std::nullopt;

			try {
				std::string jsonStr(data.begin(), data.end());
				nlohmann::ordered_json j = nlohmann::ordered_json::parse(jsonStr);
				ConnectedSystemsAPI::DataModels::Observation obs;
				from_json(j, obs);

				return obs;
			}
			catch (...) {
				// Return nullopt on any parsing error
				return std::nullopt;
			}
		}

		/// <summary>
		/// The timestamp of the observation in milliseconds since epoch.
		/// </summary>
		int64_t getTimestamp() const { return timestamp; }

		/// <summary>
		/// The raw data received from the stream.
		/// </summary>
		const std::vector<uint8_t>& getData() const { return data; }

		/// <summary>
		/// The format of the data.
		/// </summary>
		Constants::RequestFormat getFormat() const { return format; }

		/// <summary>
		/// The data stream that produced this event.
		/// </summary>
		OSHDataStream* getDataStream() const { return dataStream; }
	};
}