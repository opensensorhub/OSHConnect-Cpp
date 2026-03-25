#pragma once

#include <string>

#include "Constants/Service.h"

namespace OSHConnect::Util {
	/// <summary>
	/// Utility class providing helper functions.
	/// </summary>
	class Utilities {
	public:
		/// <summary>
		/// Strips the protocol (http:// or https://) from a given URL string.
		/// </summary>
		/// <param name="url">The URL string to process.</param>
		/// <returns>The URL string without the protocol prefix.</returns>
		static std::string stripProtocol(const std::string& url) {
			if (url.find("https://") == 0) {
				return url.substr(8);
			}
			else if (url.find("http://") == 0) {
				return url.substr(7);
			}
			return url;
		}

		/// <summary>
		/// Joins path segments with appropriate slashes.
		/// </summary>
		static std::string joinPath(const std::string& base, const std::string& path) {
			if (base.empty()) return path;
			if (path.empty()) return base;
			if (base.back() == '/' && path.front() == '/') {
				return base + path.substr(1);
			}
			else if (base.back() != '/' && path.front() != '/') {
				return base + "/" + path;
			}
			else {
				return base + path;
			}
		}

		/// <summary>
		/// Joins a base path with a Service enum endpoint.
		/// </summary>
		static std::string joinPath(const std::string& base, Constants::Service service) {
			return joinPath(base, Constants::getEndpoint(service));
		}

		/// <summary>
		/// Joins multiple path segments with appropriate slashes.
		/// </summary>
		template<typename... Args>
		static std::string joinPath(const std::string& first, const std::string& second, const Args&... more) {
			if constexpr (sizeof...(more) == 0) {
				return joinPath(first, second);
			}
			else {
				return joinPath(joinPath(first, second), more...);
			}
		}
	};
}