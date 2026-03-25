#pragma once

#include <string>
#include <stdexcept>
#include <ostream>

namespace OSHConnect::Constants {
	/// <summary>
	/// Enum representing the format of the request.
	/// </summary>
	enum class RequestFormat {
		JSON,
		OM_JSON,
		SWE_CSV,
		SWE_JSON,
		SWE_XML,
		SWE_BINARY,
		PLAIN_TEXT
	};

	inline std::string getMimeType(RequestFormat rf) {
		switch (rf) {
		case RequestFormat::JSON:        return "application/json";
		case RequestFormat::OM_JSON:     return "application/om%2Bjson";
		case RequestFormat::SWE_CSV:     return "application/swe%2Bcsv";
		case RequestFormat::SWE_JSON:    return "application/swe%2Bjson";
		case RequestFormat::SWE_XML:     return "application/swe%2Bxml";
		case RequestFormat::SWE_BINARY:  return "application/swe%2Bbinary";
		case RequestFormat::PLAIN_TEXT:  return "text/plain";
		}
		throw std::invalid_argument("Unknown RequestFormat value");
	}

	inline std::ostream& operator<<(std::ostream& os, RequestFormat rf) {
		return os << getMimeType(rf);
	}

	inline std::string operator+(const std::string& lhs, RequestFormat rhs) {
		return lhs + getMimeType(rhs);
	}
}
