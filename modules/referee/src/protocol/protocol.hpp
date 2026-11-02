#pragma once

#include "parser.h"

#include <array>
#include <cstring>
#include <tuple>
#include <type_traits>

namespace breeze::referee::protocol
{

template <typename... Protocols>
class Protocol final
{
	static consteval bool UniqueIds()
	{
		constexpr std::array IDS{Protocols::ID...};
		for (std::size_t i = 0; i < IDS.size(); ++i)
		{
			for (std::size_t j = i + 1; j < IDS.size(); ++j)
			{
				if (IDS[i] == IDS[j])
				{
					return false;
				}
			}
		}
		return true;
	}

	static_assert(sizeof...(Protocols) > 0);
	static_assert((std::is_trivially_copyable_v<Protocols> && ...));
	static_assert(UniqueIds());

public:
	template <typename Handler>
	static void Dispatch(const RawPackage& package, Handler&& handler)
	{
		(DispatchOne<Protocols>(package, handler), ...);
	}

	Protocol() = delete;

private:
	template <typename T>
	static void DispatchOne(const RawPackage& package, auto& handler)
	{
		if (package.CmdId != T::ID || package.Payload.size() != sizeof(T))
		{
			return;
		}

		T value{};
		std::memcpy(&value, package.Payload.data(), sizeof(T));
		handler(value);
	}
};

} // namespace breeze::referee::protocol
