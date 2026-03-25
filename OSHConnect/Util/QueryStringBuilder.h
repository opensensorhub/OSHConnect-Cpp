#pragma once

#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <CSAPI/DataModels/TimeExtent.h>

namespace OSHConnect::Util {
	/// <summary>
	/// A utility class for building query strings for OpenSensorHub API requests.
	/// <para>
	/// Note that if the key or value of a parameter is null or empty, it will not be included in the query string.
	/// As such, it is safe to add parameters unconditionally.
	/// </para>
	/// <para>
	/// Example usage:
	/// </para>
	/// <code>
	///   auto queryString = QueryStringBuilder()
	///       .withParameter("limit", 10)
	///       .withParameter("format", "json")
	///       .build();
	/// </code>
	/// </summary>
	class QueryStringBuilder {
	private:
		/// <summary>
		/// The map of parameters.
		/// The key is the parameter name, and the value is the parameter value.
		/// This will not contain any parameters with null or empty values.
		/// </summary>
		std::map<std::string, std::string, std::less<>> parameters;

	public:
		QueryStringBuilder() = default;
		~QueryStringBuilder() = default;
		QueryStringBuilder(const QueryStringBuilder& other) = default;
		QueryStringBuilder& operator=(const QueryStringBuilder& other) = default;
		QueryStringBuilder(QueryStringBuilder&& other) noexcept = default;
		QueryStringBuilder& operator=(QueryStringBuilder&& other) noexcept = default;

		/// <summary>
		/// Create a new builder instance.
		/// </summary>
		/// <returns>A new QueryStringBuilder instance.</returns>
		static QueryStringBuilder create() {
			return QueryStringBuilder();
		}

		/// <summary>
		/// Create a new QueryStringBuilder from a map of parameters.
		/// </summary>
		/// <param name="map">The map of parameters.</param>
		/// <returns>A new QueryStringBuilder with the parameters.</returns>
		static QueryStringBuilder fromMap(const std::map<std::string, std::string, std::less<>>& map) {
			QueryStringBuilder builder;
			for (const auto& [key, value] : map) {
				builder.withParameter(key, value);
			}
			return builder;
		}

		/// <summary>
		/// Add a parameter to the query string.
		/// If the key or value is null or empty, the parameter will not be added.
		/// </summary>
		/// <param name="key">The name of the parameter.</param>
		/// <param name="value">The value of the parameter.</param>
		/// <returns>Reference to this builder for method chaining.</returns>
		QueryStringBuilder& withParameter(std::string_view key, std::string_view value) {
			if (key.empty() || value.empty()) return *this;
			parameters[std::string(key)] = std::string(value);
			return *this;
		}

		/// <summary>
		/// Add a parameter to the query string.
		/// The value will be converted to a string using stream output.
		/// </summary>
		/// <typeparam name="T">The type of the parameter value.</typeparam>
		/// <param name="key">The name of the parameter.</param>
		/// <param name="value">The value of the parameter.</param>
		/// <returns>Reference to this builder for method chaining.</returns>
		template<typename T>
		QueryStringBuilder& withParameter(std::string_view key, const T& value) {
			std::ostringstream oss;
			oss << value;
			return withParameter(key, oss.str());
		}

		/// <summary>
		/// Add a parameter with a vector of values. The values will be joined with commas.
		/// </summary>
		/// <typeparam name="T">The type of the parameter values.</typeparam>
		/// <param name="key">The name of the parameter.</param>
		/// <param name="values">The vector of values.</param>
		/// <returns>Reference to this builder for method chaining.</returns>
		template<typename T>
		QueryStringBuilder& withParameter(std::string_view key, const std::vector<T>& values) {
			if (values.empty()) return *this;

			std::ostringstream csv;
			for (size_t i = 0; i < values.size(); ++i) {
				if (i > 0) csv << ',';
				csv << values[i];
			}
			return withParameter(key, csv.str());
		}

		/// <summary>
		/// Add a time extent parameter.
		/// This will format the time extent as a string in one of the following formats:
		/// <para>
		/// - "now" if the time extent represents the special case of the current time.
		/// </para><para>
		/// - time if the time extent represents an instant.
		/// </para><para>
		/// - begin/end if the time extent represents a period of time.
		/// </para>
		/// </summary>
		/// <param name="key">The name of the parameter.</param>
		/// <param name="value">The TimeExtent value.</param>
		/// <returns>Reference to this builder for method chaining.</returns>
		QueryStringBuilder& withParameter(std::string_view key, const ConnectedSystemsAPI::DataModels::TimeExtent& value) {
			return withParameter(key, value.toStringUTC());
		}

		/// <summary>
		/// Add a parameter with an optional value.
		/// If the optional has no value, the parameter will not be added.
		/// </summary>
		/// <typeparam name="T">The type of the parameter value.</typeparam>
		/// <param name="key">The name of the parameter.</param>
		/// <param name="value">The optional value.</param>
		/// <returns>Reference to this builder for method chaining.</returns>
		template<typename T>
		QueryStringBuilder& withParameter(std::string_view key, const std::optional<T>& value) {
			if (value.has_value()) {
				return withParameter(key, value.value());
			}
			return *this;
		}

		/// <summary>
		/// Add multiple parameters from a map.
		/// </summary>
		/// <param name="map">The map of parameters to add.</param>
		/// <returns>Reference to this builder for method chaining.</returns>
		QueryStringBuilder& withParameters(const std::map<std::string, std::string, std::less<>>& map) {
			for (const auto& [key, value] : map) {
				withParameter(key, value);
			}
			return *this;
		}

		/// <summary>
		/// Clear all parameters from the builder.
		/// </summary>
		/// <returns>Reference to this builder for method chaining.</returns>
		QueryStringBuilder& clear() {
			parameters.clear();
			return *this;
		}

		/// <summary>
		/// Check if the builder has no parameters.
		/// </summary>
		/// <returns>True if empty, false otherwise.</returns>
		bool isEmpty() const {
			return parameters.empty();
		}

		/// <summary>
		/// Build and return the query string.
		/// If no parameters were added, this will return an empty string.
		/// </summary>
		/// <returns>The constructed query string.</returns>
		std::string build() const {
			if (parameters.empty()) return "";

			std::ostringstream queryString;
			queryString << '?';
			bool first = true;
			for (const auto& [key, value] : parameters) {
				if (!first) queryString << '&';
				queryString << key << '=' << value;
				first = false;
			}
			return queryString.str();
		}

		/// <summary>
		/// Get the parameters map.
		/// </summary>
		/// <returns>The parameters map.</returns>
		const std::map<std::string, std::string, std::less<>>& getParameters() const {
			return parameters;
		}

		/// <summary>
		/// Stream output operator for convenient printing.
		/// </summary>
		friend std::ostream& operator<<(std::ostream& os, const QueryStringBuilder& builder) {
			return os << builder.build();
		}

		/// <summary>
		/// Explicit conversion to std::string.
		/// </summary>
		explicit operator std::string() const {
			return build();
		}
	};
}