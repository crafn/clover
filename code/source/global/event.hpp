#ifndef CLOVER_GLOBAL_EVENT_HPP
#define CLOVER_GLOBAL_EVENT_HPP

#include "util/variant.hpp"
#include "game/worldentity_set.hpp"
#include "util/map.hpp"
#include "util/string.hpp"
#include "util/dyn_array.hpp"

#include <bitset>

namespace clover {
namespace global {

/// @todo Unclumsify
class Event {
public:
    enum EType {
        NoneEvent,

        #define EVT(a, ...) a,
        #define VAR(b)
        #include "events.def"
        #undef VAR
        #undef EVT

        LastEvent
    };

    enum VariableType {
        NoneVariable,

        #define EVT(a, ...)
        #define VAR(b) b,
        #include "events.def"
        #undef VAR
        #undef EVT
        LastVariable

    };

    static const int32 maxVariables=15;

    struct EventTypeVariables {
        VariableType vars[maxVariables];
		const VariableType& operator[](int32 i) const { return vars[i]; }
        int32 getVarCount() const;
    };

    static const EventTypeVariables eventTypeVariables[LastEvent];
    static const util::Str8 eventNames[LastEvent];
    static const util::Str8 variableNames[LastVariable];

    Event(EType t);
	/// Queues event
    virtual ~Event();

    EType getType() const;
    util::Str8 getName() const;

    /// Argument accessor
    util::Variant& operator()(VariableType s);

    bool hasVariable(VariableType s);
	bool canHaveVariable(VariableType s);

    void addReceiver(const game::WeHandle& h);
    void addReceivers(const game::WESet& h);

    int32 getReceiverCount();

    void queue();
	bool isQueued() const { return queued; }

    void send();

private:
    EType type;
    bool queued;
    game::WESet receivers;
    util::Map<VariableType, util::Variant> vars;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_EVENT_HPP
