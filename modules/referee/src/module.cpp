#include "breeze/referee/module.h"
#include "conversion/processor.hpp"
#include "protocol/parser.h"

#include <etl/array.h>
#include <etl/circular_buffer.h>

namespace breeze::referee
{
namespace
{
struct ParserState
{
	etl::circular_buffer<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_RING_BUF_SIZE> Ring{};
	etl::array<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_LINEAR_BUF_SIZE> Linear{};
};

ParserState refereeParser;
ParserState vtParser;
internal::Processor* processor = nullptr;
internal::State state;

Outputs MkOutputs(internal::State& state)
{
	return Outputs{
	    .SelfId = ramen::Pullable<models::RobotId>{[&state](auto& value) { value = state.SelfId; }},
	    .AllyPowers =
	        ramen::Pullable<models::messages::RobotPowers>{[&state](auto& value) { value = state.AllyPowers; }},
	    .GameState = ramen::Pullable<models::messages::GameState>{[&state](auto& value) { value = state.GameState; }},
	    .LogisticsInfo =
	        ramen::Pullable<models::messages::LogisticsInfo>{[&state](auto& value) { value = state.LogisticsInfo; }},
	    .DartHitEvent = ramen::Property<models::messages::DartHitEvent>{models::messages::DartHitEvent{}},
	    .DartInfo = ramen::Pullable<models::messages::DartInfo>{[&state](auto& value) { value = state.DartInfo; }},
	    .AllyHealths =
	        ramen::Pullable<models::messages::RobotHealths>{[&state](auto& value) { value = state.AllyHealths; }},
	    .AllyLevels =
	        ramen::Pullable<models::messages::RobotLevels>{[&state](auto& value) { value = state.AllyLevels; }},
	    .AllyHeats = ramen::Pullable<models::messages::RobotHeats>{[&state](auto& value) { value = state.AllyHeats; }},
	    .AllyAmmos = ramen::Pullable<models::messages::RobotAmmos>{[&state](auto& value) { value = state.AllyAmmos; }},
	    .AllyBuffs = ramen::Pullable<models::messages::RobotBuffs>{[&state](auto& value) { value = state.AllyBuffs; }},
	    .AllyPoses = ramen::Pullable<models::messages::RobotPoses>{[&state](auto& value) { value = state.AllyPoses; }},
	};
}

void OnRawPackage(RawPackage package)
{
	protocol::rx::Receiver::Dispatch(package, [](const auto& value) { processor->Receive(value); });
}

void Feed(ParserState& state, etl::span<const uint8_t> data)
{
	StreamParser::Feed(
	    state.Ring, {state.Linear.begin(), state.Linear.end()}, data, etl::make_delegate<OnRawPackage>()
	);
}
} // namespace

Module::Module()
    : Out(MkOutputs(state)),
      InReferee([](const etl::span<const uint8_t>& data) { Feed(refereeParser, data); }),
      InVT([](const etl::span<const uint8_t>& data) { Feed(vtParser, data); })
{
	static internal::Processor instance{Out, state};
	processor = &instance;
}

Module& Module::Instance()
{
	static Module instance;
	return instance;
}

Module& module = Module::Instance();

} // namespace breeze::referee
