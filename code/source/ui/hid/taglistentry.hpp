#ifndef CLOVER_UI_HID_TAGLISTENTRY_HPP
#define CLOVER_UI_HID_TAGLISTENTRY_HPP

#include "build.hpp"
#include "context.hpp"
#include "contextchannel.hpp"
#include "util/linkedlist.hpp"
#include "script/typestring.hpp"

namespace clover {
namespace ui { namespace hid {

class TagListEntry {
public:
	/// Invalid state, needed by script
	TagListEntry();

	TagListEntry(	const ContextChannel::Name& channel_name,
					const Context::Tag& tag);
	TagListEntry(	const ContextChannel::Name& channel_name,
					const Context::Tag& tag,
					const Context::Tag& super_tag);
	TagListEntry(const TagListEntry& other);
	TagListEntry(TagListEntry&& other);
	virtual ~TagListEntry();
	
	TagListEntry& operator=(const TagListEntry& other);
	TagListEntry& operator=(TagListEntry&& other);
	
	const ContextChannel::Name& getChannelName() const { return channelName; }
	const Context::Tag& getTag() const { return tag; }

	bool isSet() const;

private:
	using TagList= util::LinkedList<Context::Tag>;
	/// TagListEntries append themselves to this list, and .back() tag of
	/// every channels' list is the enabled one
	static util::Map<ContextChannel::Name, TagList> tagLists;

	TagList::Iter endIter() const;
	void appendToList();
	void removeFromList();
	
	ContextChannel::Name channelName;
	Context::Tag tag, superTag;
	TagList::Iter iterator;
};

}} // ui::hid
namespace util {

template <>
struct TypeStringTraits<ui::hid::TagListEntry> {
	static util::Str8 type(){ return "ui::hid::TagListEntry"; }	
};

} // util
} // clover

#endif // CLOVER_UI_HID_TAGLISTENTRY_HPP