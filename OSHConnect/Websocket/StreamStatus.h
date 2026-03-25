#pragma once

namespace OSHConnect::Websocket {

	/// <summary>
	/// Status of a stream connection.
	/// </summary>
	enum class StreamStatus {
		/// <summary>
		/// The stream is not connected.
		/// </summary>
		DISCONNECTED,

		/// <summary>
		/// The stream is connected and active.
		/// </summary>
		CONNECTED,

		/// <summary>
		/// The stream has been shut down and cannot be reconnected.
		/// </summary>
		SHUTDOWN
	};

}
