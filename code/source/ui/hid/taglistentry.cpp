#include "taglistentry.hpp"

namespace clover {
namespace ui { namespace hid {
util::Map<ContextChannel::Name, TagListEntry::TagList> TagListEntry::tagLists;

TagListEntry::TagListEntry(const ContextChannel::Name& channel_name_, const Context::Tag& tag_)
	: channelName(channel_name_)
	, tag(tag_)
	, iterator(endIter()){
	appendToList();
}

TagListEntry::TagListEntry(	const ContextChannel::Name& channel_name_,
							const Context::Tag& tag_,
							const Context::Tag& super_tag_)
	: channelName(channel_name_)
	, tag(tag_)
	, superTag(super_tag_)
	, iterator(endIter()){
	appendToList();
}

TagListEntry::TagListEntry(const TagListEntry& other)
	: channelName(other.channelName)
	, tag(other.tag)
	, superTag(other.superTag)
	, iterator(endIter()){
	appendToList();
}

TagListEntry::TagListEntry(TagListEntry&& other)
	: iterator(endIter()){
	*this = std::move(other);
}

TagListEntry::~TagListEntry(){
	removeFromList();
}

TagListEntry& TagListEntry::operator=(const TagListEntry& other){
	removeFromList();
	channelName= other.channelName;
	tag= other.tag;
	superTag= other.superTag;
	iterator= endIter();
	appendToList();
	
	return *this;
}

TagListEntry& TagListEntry::operator=(TagListEntry&& other){
	removeFromList();
	channelName= other.channelName;
	tag= other.tag;
	superTag= other.superTag;
	iterator= other.iterator;
	
	other.iterator= other.endIter();
	
	return *this;
}

bool TagListEntry::isSet() const {
	return iterator != endIter();
}

auto TagListEntry::endIter() const -> TagList::Iter {
	return tagLists[channelName].end();
}

void TagListEntry::appendToList(){
	ensure(iterator == endIter());

	if (tag.length() == 0)
		return;

	auto& list= tagLists[channelName];

	if (superTag.length() == 0){
		iterator= list.insert(endIter(), tag);
	}
	else {
		auto last_found= list.end();
		for (auto it= list.begin(); it != list.end(); ++it){
			if (*it == superTag)
				last_found= std::next(it);
		}

		iterator= list.insert(last_found, tag);
	}

	ensure(iterator != tagLists[channelName].end());

	if (iterator == std::prev(list.end(), 1)){
		global::Event e(global::Event::OnHidTagChange);
		e(global::Event::ContextChannelName)= channelName;
		e(global::Event::Tag)= tag;
		e.send();
	}

#if 0
	print(debug::Ch::General, debug::Vb::Critical, "Channel %s", channelName.cStr());
	for (auto&& m : tagLists[channelName]){
		print(debug::Ch::General, debug::Vb::Trivial, "	 %s", m.cStr());
	}
#endif
}

void TagListEntry::removeFromList(){
	TagList& list= tagLists[channelName];
	if (iterator != list.end()){
		list.erase(iterator);
		iterator= list.end();
		
		Context::Tag tag= ""; // Empty tag == no tag
		
		if (!list.empty())
			tag= list.back();
		
		global::Event e(global::Event::OnHidTagChange);
		e(global::Event::ContextChannelName)= channelName;
		e(global::Event::Tag)= tag;
		e.send();
	}
}

}} // ui::hid
} // clover
