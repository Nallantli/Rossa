#ifndef CNUMBER_H
#define CNUMBER_H

#include <cmath>

typedef long double long_double_t;
typedef signed long long long_int_t;

enum NumberType
{
	DOUBLE_NUM,
	LONG_NUM
};

class CNumber
{
private:
	NumberType type;
	long_double_t valueDouble;
	long_int_t valueLong;

public:
	inline void validate()
	{
		if (valueDouble == static_cast<long_int_t>(valueDouble))
		{
			valueLong = static_cast<long_int_t>(valueDouble);
			type = LONG_NUM;
		}
	}

	CNumber() : valueLong(0), valueDouble(0), type(LONG_NUM) {}

	CNumber(long_double_t valueDouble) : valueDouble(valueDouble), valueLong(0), type(DOUBLE_NUM) { validate(); }

	CNumber(long_int_t valueLong) : valueLong(valueLong), valueDouble(0), type(LONG_NUM) {}

	inline NumberType getType()
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

	inline void operator+=(long_int_t v)
	{
		switch (type)
		{
		case DOUBLE_NUM:
			valueDouble += static_cast<long_double_t>(v);
		case LONG_NUM:
			valueLong += v;
		default:
			break;
		}
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
			return std::to_string(valueDouble);
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
			return 0;
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