#pragma once

#include <string>
#include <stdexcept>
#include <ostream>

namespace OSHConnect::Constants {
	enum class Service {
		API,
		SYSTEMS,
		DATASTREAMS,
		CONTROLSTREAMS,
		OBSERVATIONS,
		COMMANDS
	};

	inline std::string getEndpoint(Service s) {
		switch (s) {
		case Service::API:           return "api";
		case Service::SYSTEMS:       return "systems";
		case Service::DATASTREAMS:   return "datastreams";
		case Service::CONTROLSTREAMS:return "controlstreams";
		case Service::OBSERVATIONS:  return "observations";
		case Service::COMMANDS:      return "commands";
		}
		throw std::invalid_argument("Unknown Service value");
	}

	inline std::ostream& operator<<(std::ostream& os, Service s) {
		return os << getEndpoint(s);
	}

	inline std::string operator+(const std::string& lhs, Service rhs) {
		return lhs + getEndpoint(rhs);
	}
}