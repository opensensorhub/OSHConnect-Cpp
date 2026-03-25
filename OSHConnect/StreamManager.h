#pragma once

#include <vector>
#include <unordered_set>
#include <memory>
#include <functional>
#include <algorithm>
#include <utility>

#include "Websocket/StreamHandler.h"
#include "Websocket/StreamEventArgs.h"

namespace OSHConnect {
	// Forward declaration
	class OSHConnect;

	/// <summary>
	/// Manages data stream handlers and their associated data streams.
	/// </summary>
	class StreamManager {
	private:
		/// <summary>
		/// Data stream handlers created and owned by this manager.
		/// </summary>
		std::vector<std::unique_ptr<Websocket::StreamHandler>> ownedHandlers;

		/// <summary>
		/// External data stream handlers added to this manager (not owned).
		/// </summary>
		std::unordered_set<Websocket::StreamHandler*> externalHandlers;

		friend class OSHConnect;

		/// <summary>
		/// Package-private constructor, to be used by OSHConnect.
		/// </summary>
		StreamManager() = default;

	public:
		~StreamManager() {
			shutdown();
		}

		// Prevent copying
		StreamManager(const StreamManager&) = delete;
		StreamManager& operator=(const StreamManager&) = delete;

		/// <summary>
		/// Create a new data stream handler and add it to OSHConnect.
		/// No need to call addDataStreamHandler after calling this method.
		/// The manager takes ownership of the created handler.
		/// </summary>
		/// <param name="onStreamUpdate">The function to call when a data stream is updated.</param>
		/// <returns>Pointer to the created data stream handler.</returns>
		Websocket::StreamHandler* createDataStreamHandler(
			std::function<void(const Websocket::StreamEventArgs&)> onStreamUpdate) {

			// Create anonymous derived class
			class CallbackStreamHandler : public Websocket::StreamHandler {
			private:
				std::function<void(const Websocket::StreamEventArgs&)> callback;

			public:
				explicit CallbackStreamHandler(
					std::function<void(const Websocket::StreamEventArgs&)> callback)
					: callback(std::move(callback)) {
				}

				void onStreamUpdate(const Websocket::StreamEventArgs& args) override {
					if (callback) {
						callback(args);
					}
				}
			};

			auto handler = std::make_unique<CallbackStreamHandler>(std::move(onStreamUpdate));
			auto* ptr = handler.get();
			ownedHandlers.push_back(std::move(handler));
			return ptr;
		}

		/// <summary>
		/// Add an external data stream handler to OSHConnect.
		/// This method is used to add a data stream handler created outside the OSHConnect instance.
		/// The caller maintains ownership of the handler.
		/// </summary>
		/// <param name="handler">The data stream handler to add.</param>
		void addDataStreamHandler(Websocket::StreamHandler* handler) {
			if (handler) {
				externalHandlers.insert(handler);
			}
		}

		/// <summary>
		/// Get a list of all data stream handlers associated with OSHConnect.
		/// This includes both owned and external handlers.
		/// </summary>
		/// <returns>A vector of pointers to data stream handlers.</returns>
		std::vector<Websocket::StreamHandler*> getDataStreamHandlers() const {
			std::vector<Websocket::StreamHandler*> result;

			// Add owned handlers
			for (const auto& handler : ownedHandlers) {
				result.push_back(handler.get());
			}

			// Add external handlers
			for (auto* handler : externalHandlers) {
				result.push_back(handler);
			}

			return result;
		}

		/// <summary>
		/// Shutdown the data stream handler and its associated data streams,
		/// and remove it from OSHConnect.
		/// If the handler was created by this manager, it will be destroyed.
		/// </summary>
		/// <param name="handler">The handler to shutdown and remove.</param>
		void shutdownDataStreamHandler(Websocket::StreamHandler* handler) {
			if (!handler) return;

			// Shutdown the handler
			handler->shutdown();

			// Remove from external handlers
			externalHandlers.erase(handler);

			// Remove from owned handlers (and destroy if owned)
			ownedHandlers.erase(
				std::remove_if(ownedHandlers.begin(), ownedHandlers.end(),
					[handler](const std::unique_ptr<Websocket::StreamHandler>& h) {
				return h.get() == handler;
			}),
				ownedHandlers.end()
			);
		}

		/// <summary>
		/// Shutdown all data stream handlers and disconnect from all data streams,
		/// and remove them from OSHConnect.
		/// </summary>
		void shutdownDataStreamHandlers() {
			// Shutdown owned handlers
			for (auto const& handler : ownedHandlers) {
				handler->shutdown();
			}
			ownedHandlers.clear();

			// Shutdown external handlers (but don't delete them)
			for (auto* handler : externalHandlers) {
				handler->shutdown();
			}
			externalHandlers.clear();
		}

	private:
		/// <summary>
		/// Shutdown all data stream handlers.
		/// Package-private method called by OSHConnect.
		/// </summary>
		void shutdown() {
			shutdownDataStreamHandlers();
		}
	};

}
