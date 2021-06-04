#ifndef NUMBER_H
#define NUMBER_H

#include <string>
#include <cmath>
#include <sstream>
#include <limits>

typedef double long_double_t;
typedef long long int long_int_t;

struct number_t
{
private:
	union
	{
		long_double_t valueDouble;
		long_int_t valueLong;
	};

	void validate() noexcept;
	number_t(const long_double_t &valueDouble) noexcept;
	number_t(const long_int_t &valueLong) noexcept;

public:
	enum
	{
		DOUBLE_NUM,
		LONG_NUM
	} type;

	number_t() noexcept;
	static const number_t Double(const long_double_t &valueDouble) noexcept;
	static const number_t Long(const long_int_t &valueLong) noexcept;
	void operator=(const number_t &n) noexcept;
	void operator+=(const number_t &n) noexcept;
	void operator-=(const number_t &n) noexcept;
	const long_double_t getDouble() const noexcept;
	const long_int_t getLong() const noexcept;
	const number_t operator+() const noexcept;
	const number_t operator-() const noexcept;
	const number_t operator+(const number_t &n) const noexcept;
	const number_t operator-(const number_t &n) const noexcept;
	const number_t operator*(const number_t &n) const noexcept;
	const number_t operator/(const number_t &n) const noexcept;
	const number_t pow(const number_t &n) const noexcept;
	const number_t operator%(const number_t &n) const noexcept;
	const bool operator==(const number_t &n) const noexcept;
	const bool operator!=(const number_t &n) const noexcept;
	const bool operator<(const number_t &n) const noexcept;
	const bool operator>(const number_t &n) const noexcept;
	const bool operator<=(const number_t &n) const noexcept;
	const bool operator>=(const number_t &n) const noexcept;
	const number_t operator&(const number_t &n) const noexcept;
	const number_t operator|(const number_t &n) const noexcept;
	const number_t operator^(const number_t &n) const noexcept;
	const number_t operator<<(const number_t &n) const noexcept;
	const number_t operator>>(const number_t &n) const noexcept;
	const number_t operator~() const noexcept;
	const std::string toString() const noexcept;
	const std::string toCodeString() const noexcept;
};

#endif