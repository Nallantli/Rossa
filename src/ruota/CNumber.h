#ifndef CNUMBER_H
#define CNUMBER_H

#include <string>
#include <cmath>
#include <sstream>

typedef long double long_double_t;
typedef signed long long long_int_t;
class CNumber
{
private:
	union
	{
		long_double_t valueDouble;
		long_int_t valueLong;
	};

	CNumber(long_double_t valueDouble) : valueDouble(valueDouble), type(DOUBLE_NUM) {}

	CNumber(long_int_t valueLong) : valueLong(valueLong), type(LONG_NUM) {}

public:
	enum
	{
		DOUBLE_NUM,
		LONG_NUM
	} type;

	CNumber() : valueLong(0), type(LONG_NUM) {}

	static inline const CNumber Double(long_double_t valueDouble)
	{
		if (valueDouble == static_cast<long_int_t>(valueDouble))
			return CNumber(static_cast<long_int_t>(valueDouble));

		return CNumber(static_cast<long_double_t>(valueDouble));
	}

	static inline const CNumber Long(long_double_t valueLong)
	{
		return CNumber(static_cast<long_int_t>(valueLong));
	}

	inline int getType() const
	{
		return type;
	}

	inline const CNumber operator+(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueDouble + n.valueDouble);
			case LONG_NUM:
				return CNumber(valueDouble + static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_double_t>(valueLong) + n.valueDouble);
			case LONG_NUM:
				return CNumber(valueLong + n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator-(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueDouble - n.valueDouble);
			case LONG_NUM:
				return CNumber(valueDouble - static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_double_t>(valueLong) - n.valueDouble);
			case LONG_NUM:
				return CNumber(valueLong - n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator*(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueDouble * n.valueDouble);
			case LONG_NUM:
				return CNumber(valueDouble * static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_double_t>(valueLong) * n.valueDouble);
			case LONG_NUM:
				return CNumber(valueLong * n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator/(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueDouble / n.valueDouble);
			case LONG_NUM:
				return CNumber(valueDouble / static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_double_t>(valueLong) / n.valueDouble);
			case LONG_NUM:
				return CNumber(static_cast<long_double_t>(valueLong) / static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber pow(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_double_t>(std::pow(valueDouble, n.valueDouble)));
			case LONG_NUM:
				return CNumber(static_cast<long_double_t>(std::pow(valueDouble, static_cast<long_double_t>(n.valueLong))));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_double_t>(std::pow(static_cast<long_double_t>(valueLong), n.valueDouble)));
			case LONG_NUM:
				return CNumber(static_cast<long_double_t>(std::pow(valueLong, n.valueLong)));
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator%(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) % static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) % n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueLong % static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(valueLong % n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline bool operator==(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueDouble == n.valueDouble;
			case LONG_NUM:
				return valueDouble == n.valueLong;
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueLong == n.valueDouble;
			case LONG_NUM:
				return valueLong == n.valueLong;
			default:
				break;
			}
		default:
			break;
		}
		return false;
	}

	inline bool operator!=(const CNumber &n) const
	{
		return !(*this == n);
	}

	inline bool operator<(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueDouble < n.valueDouble;
			case LONG_NUM:
				return valueDouble < n.valueLong;
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueLong < n.valueDouble;
			case LONG_NUM:
				return valueLong < n.valueLong;
			default:
				break;
			}
		default:
			break;
		}
		return false;
	}

	inline bool operator>(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueDouble > n.valueDouble;
			case LONG_NUM:
				return valueDouble > n.valueLong;
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueLong > n.valueDouble;
			case LONG_NUM:
				return valueLong > n.valueLong;
			default:
				break;
			}
		default:
			break;
		}
		return false;
	}

	inline bool operator<=(const CNumber &n) const
	{
		return !(*this > n);
	}

	inline bool operator>=(const CNumber &n) const
	{
		return !(*this < n);
	}

	inline const CNumber operator&(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) & static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) & n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueLong & static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(valueLong & n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator|(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) | static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) | n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueLong | static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(valueLong | n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator^(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) ^ static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) ^ n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueLong ^ static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(valueLong ^ n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator<<(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) << static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) << n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueLong << static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(valueLong << n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const CNumber operator>>(const CNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) >> static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(static_cast<long_int_t>(valueDouble) >> n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return CNumber(valueLong >> static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return CNumber(valueLong >> n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return CNumber();
	}

	inline const std::string toString() const
	{
		switch (type)
		{
		case DOUBLE_NUM:
		{
			std::stringstream ss;
			ss.precision(10);
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

	inline long_double_t getDouble() const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			return valueDouble;
		case LONG_NUM:
			return static_cast<long_double_t>(valueLong);
		default:
			return NAN;
		}
	}

	inline long_int_t getLong() const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			return static_cast<long_int_t>(valueDouble);
		case LONG_NUM:
			return valueLong;
		default:
			return 0;
		}
	}
};

#endif