#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <optional>

#include <CSAPI/DataModels/TimeExtent.h>

#include "StreamListener.h"
#include "StreamEventArgs.h"
#include "StreamStatus.h"
#include "../OSHDataStream.h"
#include "../Constants/RequestFormat.h"

namespace OSHConnect::Websocket {
	/// <summary>
	/// A handler for multiple data streams.
	/// Override the onStreamUpdate method to receive data from the data streams.
	/// This class manages multiple StreamListener instances and can time-synchronize their events.
	/// </summary>
	class StreamHandler {
	private:
		// Internal listener class that forwards events to the handler
		class ManagedStreamListener : public StreamListener {
		private:
			StreamHandler* handler;

		public:
			ManagedStreamListener(const OSHDataStream& dataStream, StreamHandler* handler)
				: StreamListener(dataStream), handler(handler) {
			}

			void onStreamUpdate(const StreamEventArgs& args) override {
				// Forward to handler (could be time-synchronized in the future)
				if (handler) {
					handler->onStreamUpdate(args);
				}
			}
		};

		std::vector<std::unique_ptr<ManagedStreamListener>> dataStreamListeners;
		std::optional<Constants::RequestFormat> requestFormat;
		std::optional<ConnectedSystemsAPI::DataModels::TimeExtent> timeExtent;
		double replaySpeed = 1.0;
		StreamStatus status = StreamStatus::DISCONNECTED;

	public:
		StreamHandler() = default;
		virtual ~StreamHandler() {
			shutdown();
		}

		// Prevent copying
		StreamHandler(const StreamHandler&) = delete;
		StreamHandler& operator=(const StreamHandler&) = delete;

		/// <summary>
		/// Virtual method called when a data stream receives an update.
		/// Override this method in derived classes to handle the incoming data.
		/// </summary>
		/// <param name="args">The event arguments containing the data and metadata.</param>
		virtual void onStreamUpdate(const StreamEventArgs& args) {
			// Override this in derived classes
		}

		/// <summary>
		/// Connects to all data streams.
		/// </summary>
		void connect() {
			if (status == StreamStatus::SHUTDOWN) {
				throw std::runtime_error("Handler has been shut down.");
			}

			for (auto& listener : dataStreamListeners) {
				listener->connect();
			}
			status = StreamStatus::CONNECTED;
		}

		/// <summary>
		/// Disconnects from all data streams.
		/// </summary>
		void disconnect() {
			for (auto& listener : dataStreamListeners) {
				listener->disconnect();
			}
			status = StreamStatus::DISCONNECTED;
		}

		/// <summary>
		/// Shuts down the data stream handler.
		/// This will disconnect from all data streams and remove them from the handler.
		/// The handler will no longer be usable after this method is called.
		/// </summary>
		void shutdown() {
			shutdownAllDataStreamListeners();
			status = StreamStatus::SHUTDOWN;
		}

		/// <summary>
		/// Adds a data stream to the handler.
		/// Also connects to the data stream if the handler is already connected.
		/// If the data stream is already in the handler, returns the existing listener.
		/// </summary>
		/// <param name="dataStream">The data stream to add.</param>
		/// <returns>Pointer to the StreamListener managing this data stream.</returns>
		StreamListener* addDataStreamListener(const OSHDataStream& dataStream) {
			// Check if already exists
			for (auto& listener : dataStreamListeners) {
				if (listener->getDataStream().getId() == dataStream.getId()) {
					return listener.get();
				}
			}

			// Create new listener
			auto listener = std::make_unique<ManagedStreamListener>(dataStream, this);

			// Apply current settings
			if (requestFormat.has_value()) {
				listener->setRequestFormat(requestFormat.value());
			}
			listener->setReplaySpeed(replaySpeed);
			if (timeExtent.has_value()) {
				listener->setTimeExtent(timeExtent.value());
			}

			// Connect if handler is already connected
			if (status == StreamStatus::CONNECTED) {
				listener->connect();
			}

			auto* ptr = listener.get();
			dataStreamListeners.push_back(std::move(listener));
			return ptr;
		}

		/// <summary>
		/// Disconnects from the data stream and removes it from the handler.
		/// </summary>
		/// <param name="dataStream">The data stream to remove.</param>
		/// <returns>True if the listener was shut down and removed, false if not found.</returns>
		bool shutdownDataStreamListener(const OSHDataStream& dataStream) {
			auto it = std::find_if(dataStreamListeners.begin(), dataStreamListeners.end(),
				[&dataStream](const std::unique_ptr<ManagedStreamListener>& listener) {
				return listener->getDataStream().getId() == dataStream.getId();
			});

			if (it != dataStreamListeners.end()) {
				(*it)->disconnect();
				dataStreamListeners.erase(it);
				return true;
			}
			return false;
		}

		/// <summary>
		/// Disconnects from the data stream and removes it from the handler.
		/// </summary>
		/// <param name="listener">The listener to remove.</param>
		/// <returns>True if the listener was removed, false if not found.</returns>
		bool shutdownDataStreamListener(StreamListener* listener) {
			if (!listener) return false;

			auto it = std::find_if(dataStreamListeners.begin(), dataStreamListeners.end(),
				[listener](const std::unique_ptr<ManagedStreamListener>& l) {
				return l.get() == listener;
			});

			if (it != dataStreamListeners.end()) {
				(*it)->disconnect();
				dataStreamListeners.erase(it);
				return true;
			}
			return false;
		}

		/// <summary>
		/// Shuts down all data streams and removes them from the handler.
		/// </summary>
		void shutdownAllDataStreamListeners() {
			for (auto& listener : dataStreamListeners) {
				listener->disconnect();
			}
			dataStreamListeners.clear();
		}

		/// <summary>
		/// Get the number of data stream listeners in the handler.
		/// </summary>
		size_t getDataStreamListenerCount() const {
			return dataStreamListeners.size();
		}

		/// <summary>
		/// The format of the request.
		/// If nullopt, the format will not be specified in the request.
		/// </summary>
		std::optional<Constants::RequestFormat> getRequestFormat() const {
			return requestFormat;
		}

		/// <summary>
		/// Sets the format of the request for all listeners.
		/// Calling this will reconnect any connected listeners.
		/// </summary>
		void setRequestFormat(Constants::RequestFormat format) {
			requestFormat = format;
			for (auto& listener : dataStreamListeners) {
				listener->setRequestFormat(format);
			}
		}

		/// <summary>
		/// The time period for the data streams.
		/// If nullopt, will listen to the data streams in real-time.
		/// </summary>
		std::optional<ConnectedSystemsAPI::DataModels::TimeExtent> getTimeExtent() const {
			return timeExtent;
		}

		/// <summary>
		/// Sets the time period for all listeners.
		/// Calling this will reconnect any connected listeners.
		/// </summary>
		void setTimeExtent(const ConnectedSystemsAPI::DataModels::TimeExtent& extent) {
			timeExtent = extent;
			for (auto& listener : dataStreamListeners) {
				listener->setTimeExtent(extent);
			}
		}

		/// <summary>
		/// The replay speed for the data streams.
		/// Only applicable for historical data streams.
		/// 1.0 is default speed, 0.1 is 10x slower, 10.0 is 10x faster.
		/// </summary>
		double getReplaySpeed() const {
			return replaySpeed;
		}

		/// <summary>
		/// Sets the replay speed for all listeners.
		/// Calling this will reconnect any connected listeners.
		/// </summary>
		void setReplaySpeed(double speed) {
			replaySpeed = speed;
			for (auto& listener : dataStreamListeners) {
				listener->setReplaySpeed(speed);
			}
		}

		/// <summary>
		/// The status of the data stream handler.
		/// </summary>
		StreamStatus getStatus() const {
			return status;
		}
	};

}
