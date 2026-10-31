#include "breeze/referee/module.h"
#include "conversion.hpp"
#include "protocol/parser.h"
#include "protocol/rx_types.hpp"

#include <etl/array.h>
#include <etl/circular_buffer.h>
#include <utility>

namespace breeze::referee
{
namespace
{
struct ParserState
{
	etl::circular_buffer<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_RING_BUF_SIZE> Ring{};
	etl::array<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_LINEAR_BUF_SIZE> Linear{};
};

struct Pipeline
{
	ParserState RefereeParser{};
	ParserState VTParser{};
	protocol::rx::Receiver Receiver{};
	internal::Conversion Conversion{};

	Pipeline()
	{
		Connect(std::make_index_sequence<std::tuple_size_v<protocol::rx::Types>>{});
	}

	template <std::size_t... Is>
	void Connect(std::index_sequence<Is...>)
	{
		((std::get<Is>(Receiver.Out) >> std::get<Is>(Conversion.In)), ...);
	}
};

Pipeline pipeline;

void OnRawPackage(RawPackage package) { pipeline.Receiver.In(package); }

template <typename Ring>
void Feed(ParserState& state, Ring& ring, etl::span<const uint8_t> data)
{
	StreamParser::Feed(ring, {state.Linear.begin(), state.Linear.end()}, data, etl::make_delegate<OnRawPackage>());
}

} // namespace

Module::Module()
	: OutSelfId([](models::RobotId& value) { pipeline.Conversion.OutSelfId(value); }),
	  OutAllyPowers([](models::messages::RobotPowers& value) { pipeline.Conversion.OutAllyPowers(value); }),
	  OutGamestate([](models::messages::GameState& value) { pipeline.Conversion.OutGamestate(value); }),
	  OutLogisticsInfo([](models::messages::LogisticsInfo& value) { pipeline.Conversion.OutLogisticsInfo(value); }),
	  OutDartInfo([](models::messages::DartInfo& value) { pipeline.Conversion.OutDartInfo(value); }),
	  OutAllyHealths([](models::messages::RobotHealths& value) { pipeline.Conversion.OutAllyHealths(value); }),
	  OutAllyLevels([](models::messages::RobotLevels& value) { pipeline.Conversion.OutAllyLevels(value); }),
	  OutAllyHeats([](models::messages::RobotHeats& value) { pipeline.Conversion.OutAllyHeats(value); }),
	  OutAllyAmmos([](models::messages::RobotAmmos& value) { pipeline.Conversion.OutAllyAmmos(value); }),
	  OutAllyBuffs([](models::messages::RobotBuffs& value) { pipeline.Conversion.OutAllyBuffs(value); }),
	  OutAllyPoses([](models::messages::RobotPoses& value) { pipeline.Conversion.OutAllyPoses(value); }),
	  InReferee([](const etl::span<const uint8_t>& data) {
		  Feed(pipeline.RefereeParser, pipeline.RefereeParser.Ring, data);
	  }),
	  InVT([](const etl::span<const uint8_t>& data) {
		  Feed(pipeline.VTParser, pipeline.VTParser.Ring, data);
	  })
{
	pipeline.Conversion.OutPowermanagementEvent >> OutPowermanagementEvent;
	pipeline.Conversion.OutRegionEvent >> OutRegionEvent;
	pipeline.Conversion.OutDamageEvent >> OutDamageEvent;
	pipeline.Conversion.OutOccupationEvent >> OutOccupationEvent;
	pipeline.Conversion.OutRuneEvent >> OutRuneEvent;
	pipeline.Conversion.OutDartHitEvent >> OutDartHitEvent;
}

Module module;

} // namespace breeze::referee
