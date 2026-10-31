#pragma once

#include "parser.h"

#include <array>
#include <cstring>
#include <ramen.hpp>
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
	std::tuple<ramen::Pusher<Protocols>...> Out{};
	ramen::Pushable<RawPackage> In = [this](const RawPackage& package) { Dispatch(package); };

	Protocol() = default;
	Protocol(const Protocol&) = delete;
	Protocol(Protocol&&) = delete;
	Protocol& operator=(const Protocol&) = delete;
	Protocol& operator=(Protocol&&) = delete;

	template <typename T>
	ramen::Pusher<T>& OutOf()
	{
		return std::get<ramen::Pusher<T>>(Out);
	}

private:
	template <typename T>
	void DispatchOne(const RawPackage& package)
	{
		if (package.CmdId != T::ID || package.Payload.size() != sizeof(T))
		{
			return;
		}

		T value{};
		std::memcpy(&value, package.Payload.data(), sizeof(T));
		std::get<ramen::Pusher<T>>(Out)(value);
	}

	void Dispatch(const RawPackage& package) { (DispatchOne<Protocols>(package), ...); }
};

} // namespace breeze::referee::protocol
