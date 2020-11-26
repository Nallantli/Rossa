#pragma once

#include <string>
#include <cmath>
#include <sstream>
#include <limits>

typedef double long_double_t;
typedef long long int long_int_t;

class RNumber
{
private:
	union
	{
		long_double_t valueDouble;
		long_int_t valueLong;
	};

	inline void validate() noexcept
	{
		if (valueDouble == static_cast<long_int_t>(valueDouble)) {
			valueLong = valueDouble;
			type = LONG_NUM;
		}
	}

	RNumber(const long_double_t &valueDouble) noexcept : valueDouble(valueDouble), type(DOUBLE_NUM)
	{
		validate();
	}

	RNumber(const long_int_t &valueLong) noexcept : valueLong(valueLong), type(LONG_NUM)
	{}

public:
	enum
	{
		DOUBLE_NUM,
		LONG_NUM
	} type;

	RNumber() noexcept : valueLong(0), type(LONG_NUM)
	{}

	static inline const RNumber Double(const long_double_t &valueDouble) noexcept
	{
		return RNumber(valueDouble);
	}

	static inline const RNumber Long(const long_int_t &valueLong) noexcept
	{
		return RNumber(valueLong);
	}

	inline void operator=(const RNumber &n) noexcept
	{
		type = n.type;
		switch (type) {
			case DOUBLE_NUM:
				valueDouble = n.valueDouble;
				break;
			case LONG_NUM:
				valueLong = n.valueLong;
				break;
		}
	}

	inline void operator+=(const RNumber &n) noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				valueDouble += n.getDouble();
				validate();
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						valueDouble = static_cast<long_double_t>(valueLong) + n.valueDouble;
						type = DOUBLE_NUM;
						validate();
						break;
					case LONG_NUM:
						valueLong += n.valueLong;
						break;
				}
		}
	}

	inline void operator-=(const RNumber &n) noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				valueDouble -= n.getDouble();
				validate();
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						valueDouble = static_cast<long_double_t>(valueLong) - n.valueDouble;
						type = DOUBLE_NUM;
						validate();
						break;
					case LONG_NUM:
						valueLong -= n.valueLong;
						break;
				}
		}
	}

	inline const long_double_t getDouble() const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				return valueDouble;
			case LONG_NUM:
				return static_cast<long_double_t>(valueLong);
			default:
				return NAN;
		}
	}

	inline const long_int_t getLong() const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				return static_cast<long_int_t>(valueDouble);
			case LONG_NUM:
				return valueLong;
			default:
				return 0;
		}
	}

	inline const RNumber operator+(const RNumber &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(valueDouble + n.valueDouble);
					case LONG_NUM:
						return RNumber::Double(valueDouble + static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(static_cast<long_double_t>(valueLong) + n.valueDouble);
					case LONG_NUM:
						return RNumber::Long(valueLong + n.valueLong);
				}
		}
		return RNumber();
	}

	inline const RNumber operator-(const RNumber &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(valueDouble - n.valueDouble);
					case LONG_NUM:
						return RNumber::Double(valueDouble - static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(static_cast<long_double_t>(valueLong) - n.valueDouble);
					case LONG_NUM:
						return RNumber::Long(valueLong - n.valueLong);
				}
		}
		return RNumber();
	}

	inline const RNumber operator*(const RNumber &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(valueDouble * n.valueDouble);
					case LONG_NUM:
						return RNumber::Double(valueDouble * static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(static_cast<long_double_t>(valueLong) * n.valueDouble);
					case LONG_NUM:
						return RNumber::Long(valueLong * n.valueLong);
				}
		}
		return RNumber();
	}

	inline const RNumber operator/(const RNumber &n) const noexcept
	{
		if (n.getDouble() == 0)
			return RNumber::Double(INFINITY);

		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(valueDouble / n.valueDouble);
					case LONG_NUM:
						return RNumber::Double(valueDouble / static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(static_cast<long_double_t>(valueLong) / n.valueDouble);
					case LONG_NUM:
						if (valueLong % n.valueLong == 0)
							return RNumber::Long(valueLong / n.valueLong);
						else
							return RNumber::Double(static_cast<long_double_t>(valueLong) / static_cast<long_double_t>(n.valueLong));
				}
		}
		return RNumber();
	}

	inline const RNumber pow(const RNumber &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(std::pow(valueDouble, n.valueDouble));
					case LONG_NUM:
						return RNumber::Double(std::pow(valueDouble, static_cast<long_double_t>(n.valueLong)));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(std::pow(static_cast<long_double_t>(valueLong), n.valueDouble));
					case LONG_NUM:
						return RNumber::Double(std::pow(valueLong, n.valueLong));
				}
		}
		return RNumber();
	}

	inline const RNumber operator%(const RNumber &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(std::fmod(valueDouble, n.valueDouble));
					case LONG_NUM:
						return RNumber::Double(std::fmod(valueDouble, static_cast<long_double_t>(n.valueLong)));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return RNumber::Double(std::fmod(static_cast<long_double_t>(valueLong), n.valueDouble));
					case LONG_NUM:
						return RNumber::Long(valueLong % n.valueLong);
				}
		}
		return RNumber();
	}

	inline const bool operator==(const RNumber &n) const noexcept
	{
		if (type != n.type)
			return false;
		switch (type) {
			case DOUBLE_NUM:
				return valueDouble == n.valueDouble;
			case LONG_NUM:
				return valueLong == n.valueLong;
		}
		return false;
	}

	inline const bool operator!=(const RNumber &n) const noexcept
	{
		return !(*this == n);
	}

	inline const bool operator<(const RNumber &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return valueDouble < n.valueDouble;
					case LONG_NUM:
						return valueDouble < n.valueLong;
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return valueLong < n.valueDouble;
					case LONG_NUM:
						return valueLong < n.valueLong;
				}
		}
		return false;
	}

	inline const bool operator>(const RNumber &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return valueDouble > n.valueDouble;
					case LONG_NUM:
						return valueDouble > n.valueLong;
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return valueLong > n.valueDouble;
					case LONG_NUM:
						return valueLong > n.valueLong;
				}
		}
		return false;
	}

	inline const bool operator<=(const RNumber &n) const noexcept
	{
		return !(*this > n);
	}

	inline const bool operator>=(const RNumber &n) const noexcept
	{
		return !(*this < n);
	}

	inline const RNumber operator&(const RNumber &n) const noexcept
	{
		return RNumber::Long(getLong() & n.getLong());
	}

	inline const RNumber operator|(const RNumber &n) const noexcept
	{
		return RNumber::Long(getLong() | n.getLong());
	}

	inline const RNumber operator^(const RNumber &n) const noexcept
	{
		return RNumber::Long(getLong() ^ n.getLong());
	}

	inline const RNumber operator<<(const RNumber &n) const noexcept
	{
		return RNumber::Long(getLong() << n.getLong());
	}

	inline const RNumber operator>>(const RNumber &n) const noexcept
	{
		return RNumber::Long(getLong() >> n.getLong());
	}

	inline const std::string toString() const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
			{
				std::stringstream ss;
				ss.precision(std::numeric_limits<long_double_t>::digits10);
				ss << std::fixed << valueDouble;
				std::string ret = ss.str();
				while (ret.back() == '0')
					ret.pop_back();
				if (ret.back() == '.')
					ret.pop_back();
				return ret;
			}
			case LONG_NUM:
				return std::to_string(valueLong);
			default:
				return "<undefined>";
		}
	}

	inline const std::string toCodeString() const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				return "RNumber::Double(" + std::to_string(valueDouble) + ")";
			case LONG_NUM:
				return "RNumber::Long(" + std::to_string(valueLong) + ")";
			default:
				return "<undefined>";
		}
	}
};