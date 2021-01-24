#pragma once

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

	inline void validate() noexcept
	{
		if (valueDouble == static_cast<long_int_t>(valueDouble)) {
			valueLong = valueDouble;
			type = LONG_NUM;
		}
	}

	number_t(const long_double_t &valueDouble) noexcept : valueDouble(valueDouble), type(DOUBLE_NUM)
	{
		validate();
	}

	number_t(const long_int_t &valueLong) noexcept : valueLong(valueLong), type(LONG_NUM)
	{}

public:
	enum
	{
		DOUBLE_NUM,
		LONG_NUM
	} type;

	number_t() noexcept : valueLong(0), type(LONG_NUM)
	{}

	static inline const number_t Double(const long_double_t &valueDouble) noexcept
	{
		return number_t(valueDouble);
	}

	static inline const number_t Long(const long_int_t &valueLong) noexcept
	{
		return number_t(valueLong);
	}

	inline void operator=(const number_t &n) noexcept
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

	inline void operator+=(const number_t &n) noexcept
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

	inline void operator-=(const number_t &n) noexcept
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

	inline const number_t operator+() const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				return number_t::Double(valueDouble);
			case LONG_NUM:
				return number_t::Long(valueLong);
		}
		return number_t();
	}

	inline const number_t operator-() const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				return number_t::Double(-valueDouble);
			case LONG_NUM:
				return number_t::Long(-valueLong);
		}
		return number_t();
	}

	inline const number_t operator+(const number_t &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(valueDouble + n.valueDouble);
					case LONG_NUM:
						return number_t::Double(valueDouble + static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(static_cast<long_double_t>(valueLong) + n.valueDouble);
					case LONG_NUM:
						return number_t::Long(valueLong + n.valueLong);
				}
		}
		return number_t();
	}

	inline const number_t operator-(const number_t &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(valueDouble - n.valueDouble);
					case LONG_NUM:
						return number_t::Double(valueDouble - static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(static_cast<long_double_t>(valueLong) - n.valueDouble);
					case LONG_NUM:
						return number_t::Long(valueLong - n.valueLong);
				}
		}
		return number_t();
	}

	inline const number_t operator*(const number_t &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(valueDouble * n.valueDouble);
					case LONG_NUM:
						return number_t::Double(valueDouble * static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(static_cast<long_double_t>(valueLong) * n.valueDouble);
					case LONG_NUM:
						return number_t::Long(valueLong * n.valueLong);
				}
		}
		return number_t();
	}

	inline const number_t operator/(const number_t &n) const noexcept
	{
		if (n.getDouble() == 0)
			return number_t::Double(INFINITY);

		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(valueDouble / n.valueDouble);
					case LONG_NUM:
						return number_t::Double(valueDouble / static_cast<long_double_t>(n.valueLong));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(static_cast<long_double_t>(valueLong) / n.valueDouble);
					case LONG_NUM:
						if (valueLong % n.valueLong == 0)
							return number_t::Long(valueLong / n.valueLong);
						else
							return number_t::Double(static_cast<long_double_t>(valueLong) / static_cast<long_double_t>(n.valueLong));
				}
		}
		return number_t();
	}

	inline const number_t pow(const number_t &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(std::pow(valueDouble, n.valueDouble));
					case LONG_NUM:
						return number_t::Double(std::pow(valueDouble, static_cast<long_double_t>(n.valueLong)));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(std::pow(static_cast<long_double_t>(valueLong), n.valueDouble));
					case LONG_NUM:
						return number_t::Double(std::pow(valueLong, n.valueLong));
				}
		}
		return number_t();
	}

	inline const number_t operator%(const number_t &n) const noexcept
	{
		switch (type) {
			case DOUBLE_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(std::fmod(valueDouble, n.valueDouble));
					case LONG_NUM:
						return number_t::Double(std::fmod(valueDouble, static_cast<long_double_t>(n.valueLong)));
				}
				break;
			case LONG_NUM:
				switch (n.type) {
					case DOUBLE_NUM:
						return number_t::Double(std::fmod(static_cast<long_double_t>(valueLong), n.valueDouble));
					case LONG_NUM:
						return number_t::Long(valueLong % n.valueLong);
				}
		}
		return number_t();
	}

	inline const bool operator==(const number_t &n) const noexcept
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

	inline const bool operator!=(const number_t &n) const noexcept
	{
		return !(*this == n);
	}

	inline const bool operator<(const number_t &n) const noexcept
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

	inline const bool operator>(const number_t &n) const noexcept
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

	inline const bool operator<=(const number_t &n) const noexcept
	{
		return !(*this > n);
	}

	inline const bool operator>=(const number_t &n) const noexcept
	{
		return !(*this < n);
	}

	inline const number_t operator&(const number_t &n) const noexcept
	{
		return number_t::Long(getLong() & n.getLong());
	}

	inline const number_t operator|(const number_t &n) const noexcept
	{
		return number_t::Long(getLong() | n.getLong());
	}

	inline const number_t operator^(const number_t &n) const noexcept
	{
		return number_t::Long(getLong() ^ n.getLong());
	}

	inline const number_t operator<<(const number_t &n) const noexcept
	{
		return number_t::Long(getLong() << n.getLong());
	}

	inline const number_t operator>>(const number_t &n) const noexcept
	{
		return number_t::Long(getLong() >> n.getLong());
	}

	inline const number_t operator~() const noexcept
	{
		return number_t::Long(~getLong());
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
				return "number_t::Double(" + std::to_string(valueDouble) + ")";
			case LONG_NUM:
				return "number_t::Long(" + std::to_string(valueLong) + ")";
			default:
				return "<undefined>";
		}
	}
};