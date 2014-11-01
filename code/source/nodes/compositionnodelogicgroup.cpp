#include "compositionnodelogicgroup.hpp"
#include "baseoutputslot.hpp"
#include "nodetype.hpp"
#include "nodeinstancegroup.hpp"
#include "resources/cache.hpp"
#include "script/module.hpp"
#include "util/map.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>

namespace clover {
namespace nodes {

CompositionNodeLogicGroup::CompositionNodeLogicGroup():
	sorted(false),
	clearingNodes(false),
	INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", ""),
	INIT_RESOURCE_ATTRIBUTE(fileAttribute, "file", ""){
	
	fileAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			print(debug::Ch::Dev, debug::Vb::Trivial, "CompositonNodeLogiGroup fileAttribute changed: %s", fileAttribute.get().relative().cStr());
			clearNodes();
			setResourceState(State::Unloaded); // Reload if changed
		}
	});
	
	fileAttribute.get().setOnFileChangeCallback([&] (resources::PathAttributeValue::FileEvent f){
		if (getResourceState() != State::Uninit)
			print(debug::Ch::Resources, debug::Vb::Moderate, "NodeGroup file reloading is not supported!");
			//throw global::Exception("NodeGroup file modifying not supported!");
	});
	
}

CompositionNodeLogic& CompositionNodeLogicGroup::add(const NodeType& type){
	
	nodes.pushBack(CompositionNodeLogicPtr(type.createCompositionLogic(context)));
	nodes.back()->setOwner(*this);

	// Update resource state when node changes
	nodeListener.listen(*nodes.back(), [&] () {
		sorted= false;
		
		if (clearingNodes) // Do the right thing where clearing happens, not here
			return;
		
		bool all_loaded= true;
		
		for (auto& m : nodes){
			if (m->getType().getResourceState() != State::Loaded){
				all_loaded= false;
				break;
			}
		}
		
		if (all_loaded && getResourceState() != State::Loaded)
			setResourceState(State::Loaded);
		else if (!all_loaded && getResourceState() != State::Error)
			setResourceState(State::Error);
		else
			util::OnChangeCb::trigger(); // must be called separately if resource state doesn't change but some node has changed
	});
	
	sorted= false;
	
	bool nodes_ok= true;
	for (const auto& m : nodes){
		if (m->getType().getResourceState() != State::Loaded){
			nodes_ok= false;
			break;
		}
	}
	
	if (nodes_ok)
		setResourceState(State::Loaded);
	
	util::OnChangeCb::trigger();
	
	return *nodes.back();
}

void CompositionNodeLogicGroup::remove(const CompositionNodeLogic& node){
	clearingNodes= true;
	bool found= false;
	for (auto it= nodes.begin(); it != nodes.end(); ++it){
		if (&node == it->get()){
			nodes.erase(it); // Emits onChange callback
			found= true;
			break;
		}
	}
	clearingNodes= false;
	
	if (!found)
		throw global::Exception("Node not found");
	
	util::OnChangeCb::trigger();
	
}

NodeInstanceGroup* CompositionNodeLogicGroup::instantiate() const {
	if (!sorted)
		sort();
	
	NodeInstanceGroup* group= new NodeInstanceGroup(*this);
	
	return group;
}

const CompositionNodeLogic& CompositionNodeLogicGroup::getNode(const util::Str8& name) const {
	for (auto& m : nodes){
		if (m->getType().getName() == name)
			return *m;
	}
	
	throw resources::ResourceException("CompositionNodeLogicGroup: %s, Node not found: %s", getName().cStr(), name.cStr());
}

void CompositionNodeLogicGroup::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		
		if (getResourceState() == State::Uninit && fileAttribute.get().relative().length() == 0){
			setDefaultPath();
		}
		
		try {
			loadFromFile();
			
			/// @todo Check node resource states
			
			setResourceState(State::Loaded);
		}
		catch (global::Exception& e){
			// Allow nodes stay even when there's error with deserialization
			setResourceState(State::Error);
		}
	}
	else {
		clearNodes();
		setResourceState(State::Unloaded);
	}
}

void CompositionNodeLogicGroup::createErrorResource(){
	clearNodes();
	setResourceState(State::Error);
}

void CompositionNodeLogicGroup::onResourceWrite(){
	if (fileAttribute.get().relative().empty())
		setDefaultPath();

	// Save changes to node configuration when the resource is written
	saveToFile();
}

void CompositionNodeLogicGroup::sort() const {
	if (nodes.empty()){
		sorted= true;
		return;
	}
	
	// Rules:
	// - Prioritized nodes will be first in update-line
	// - Nodes with no inputs attached follow the first. Those are start points for paths
	// - Order of nodes in different paths is arbitrary (related to each other)
	// - At every crossing of possible signal paths every preceding node should be sorted to be before the crossing node
	
	// Algorithm
	// 1. Find the start nodes of paths
	// 2. Iterate over nodes to find every possible signal path
	// 3. Remove shortcuts (paths which are equivalent, but in which some node(s) are skipped over)
	// 4. Merge crossings of every path
	// 5. Iterate over the merged path (at crossings, wait for every other path which has the same crossing)
	
	util::Map<const CompositionNodeLogic*, SizeType> nodeMap;
	for (SizeType i=0; i<nodes.size(); ++i){
		nodeMap[nodes[i].get()]= i;
	}
	
	//
	// Find starting nodes for paths
	//
	
	typedef util::DynArray<const CompositionNodeLogic*> Path;
	typedef util::DynArray<Path> Paths;
	Paths paths;
	
	auto get_next_nodes = [&] (const CompositionNodeLogic& head) -> Path {
		util::DynArray<const CompositionNodeLogic*> ret;
	
		for (auto& out_slot : head.getSlots()){
			if (!out_slot->isAttached() || out_slot->isInput()) continue;
			
			for (auto& in_slot : out_slot->getAttachedSlots()){
				if (ret.find(&in_slot->getOwner()) == ret.end()){
					ret.pushBack(&in_slot->getOwner());
				}
			}
		}
		
		return (ret);
	};
	
	std::function<void (SizeType path_index, const CompositionNodeLogic& head)>
	recursive_path_search = [&] (SizeType path_index, const CompositionNodeLogic& head){
		paths[path_index].pushBack(&head);

		const Path& next_nodes= get_next_nodes(head);
		for (auto& next : next_nodes){
			
			bool loop_detected= false;
			for (auto& node_before : paths[path_index]){
				// Loop
				if (node_before == next){
					loop_detected= true;
					break;
				}
			}
			
			if (loop_detected) continue;

			if (next == next_nodes.back()){
				// Forward to next
				recursive_path_search(path_index, *next);
			}
			else {
				// Branch
				paths.resize(paths.size()+1);
				paths.back() = paths[path_index]; // Copy beginning
				recursive_path_search(paths.size()-1, *next);
			}
			
		}
	};
	
	for (auto& node : nodes){
		
		bool is_start= false;
		bool put_first= false;
		
		if (node->isUpdateRouteStart()){
			is_start= true;
			put_first= true;
		}
		else {
			is_start= true;
			
			for (const auto& input : node->getSlots()){
				if (input->isInput() && input->isAttached()){

					is_start= false;

					// Node is still start for a path if the input is connected to a output of the same node
					auto attached=  input->getAttachedSlots();
					for (const auto& m : attached){
						if (&m->getOwner() == node.get()){
							
							is_start= true;
							break;
						}
					}

					break;
				}
			}
		}
		
		if (is_start){
			if (put_first){
				// New path to first
				paths.insert(paths.begin(), Path());
				recursive_path_search(0, *node);
				ensure(!paths.front().empty());
			}
			else {
				// New path to last
				paths.insert(paths.end(), Path());
				recursive_path_search(paths.size()-1, *node);
				ensure(!paths.back().empty());
			}
		}
	}
	
	//
	// Remove shortcuts
	//
	
	auto path_includes_other = [&] (const Path& p1, const Path& p2) -> bool {
		Path::cIter cur_node_it= p1.begin();
		for (auto& node : p2){
			while (*cur_node_it != node){
				++cur_node_it;
				if (cur_node_it == p1.end())
					return false;
			}
		}
		return true;
	};
	
	for (auto path_it= paths.begin(); path_it != paths.end();){
		bool removed= false;
		
		for (auto comparison_path_it= path_it+1; comparison_path_it != paths.end(); ++comparison_path_it){
			if (path_includes_other(*comparison_path_it, *path_it)){
				path_it= paths.erase(path_it);
				removed= true;
				break;
			}
		}
		
		if (!removed)
			++path_it;
	}
	
#if 0
	for (Path& path : paths){
		print(debug::Ch::General, debug::Vb::Trivial, "---");
		for (auto& node : path){
			print(debug::Ch::General, debug::Vb::Trivial, "%s, %p", node->getType().getName().cStr(), node);
		}
	}
#endif
	
	//
	// "Merge" crossings
	//
	
	struct PathCrossing {
		Paths::Iter pathIt;
		Path::Iter nodeIt;
	};
	
	util::Map<const CompositionNodeLogic*, util::DynArray<PathCrossing>> pathCrossings;
	
	for (auto path_it = paths.begin(); path_it != paths.end(); ++path_it){
		for (auto& node : nodes){
			for (auto 	node_it = path_it->begin();
						node_it != path_it->end();
						++node_it){
				if (node.get() == *node_it){
#if 0
					print(debug::Ch::General, debug::Vb::Trivial, "pathCrossing added for: %s, %p", node->getType().getName().cStr(), node.get());
#endif
					pathCrossings[node.get()].pushBack({path_it, node_it});
				}
			}
		}
	}
	
	//
	// Traverse through paths
	//
	
	util::DynArray<SizeType> sorted_indices; // Indices to nodes -array
	
	typedef PathCrossing Head;
	
	// Create initial heads
	util::DynArray<Head> heads;
	for (auto path_it = paths.begin(); path_it != paths.end(); ++path_it){
		heads.pushBack({path_it, path_it->begin()});
	}
	
	util::DynArray<Head*> active_heads;
	for (auto& m : heads){
		active_heads.pushBack(&m);
	}
	
	auto is_sorted = [&] (const CompositionNodeLogic& node) -> bool {
		return sorted_indices.find(nodeMap[&node]) != sorted_indices.end();
	};
	
	auto try_add_sorted = [&] (const Head& h) {
		if (!is_sorted(*(*h.nodeIt))){
			sorted_indices.pushBack(nodeMap[*h.nodeIt]);
		}
	};
	
	// True if every head which has the crossing in which h is, is at the crossing
	auto is_advancing_allowed = [&] (const Head& h) -> bool {
		if (h.nodeIt == h.pathIt->end())
			return false;
		
		ensure_msg(	pathCrossings.find(*h.nodeIt) != pathCrossings.end(),
					"%s, %p", (*h.nodeIt)->getType().getName().cStr(), *h.nodeIt);
		
		SizeType crossing_count= pathCrossings[*h.nodeIt].size();
		SizeType cur_count= 0;
		for (auto& m : heads){

			if (*m.nodeIt == *h.nodeIt)
				++cur_count;
		}

		ensure(crossing_count > 0);
		
		return cur_count == crossing_count || is_sorted(*(*h.nodeIt)) || (*h.nodeIt)->isUpdateRouteStart();
	};

	auto advance = [&] (Head& h) {
		try_add_sorted(h);
		ensure(h.nodeIt != h.pathIt->end());
		++h.nodeIt;
	};

	while (!active_heads.empty()){
		bool advanced= false;

		// Advance every head as far as possible
		for (auto head_it= active_heads.begin(); head_it != active_heads.end();){
			Head& head= *(*head_it);
			if (is_advancing_allowed(head)){
				advance(head);
				advanced= true;
			}
			
			// Dead end
			if (head.nodeIt == head.pathIt->end()){
				head_it= active_heads.erase(head_it);
				advanced= true;
				continue;
			}

			++head_it;
		}


		// Deadlock! (simplest case: ABCB, DCBC)
		if (!advanced){
			// Trivial fix:
			// Advance first paths first (e.g. path starting from WeInterface)
			// Causes delays in signal paths when updating nodes, but it's inevitable

			for (auto& m : active_heads){
				if (m->nodeIt != m->pathIt->end()){
					advance(*m);
					break;
				}
			}
		}
	}
	
	//
	// Apply sorting
	//
	
	util::DynArray<CompositionNodeLogicPtr> sorted_final;
	
	for (auto& m : sorted_indices){
		sorted_final.pushBack(std::move(nodes[m]));
	}

	ensure_msg(nodes.size() == sorted_final.size(), "nodes.size(): %zu, sorted_final.size(): %zu", nodes.size(), sorted_final.size());
	nodes= std::move(sorted_final);
	sorted= true;
}


void CompositionNodeLogicGroup::clearNodes(){
	clearingNodes= true;
	nodes.clear(); // Emits all kind of onChange callbacks because nodes are changed when destroyed
	clearingNodes= false;
}

void CompositionNodeLogicGroup::setDefaultPath(){
	// *.ng file automatically to the same folder with the same name than resource file itself (but lowercased)
	fileAttribute.set(resources::PathAttributeValue(getResourceFilePath().directoryFromRoot(), "./" + nameAttribute.get().lowerCased() + ".ng"));
}

//
// Serialization
//

struct CompositionNodeLogicGroup::SerializationGraph {
	typedef uint32 NodeId; // Index in array
	
	struct Node {
		
		struct Input {
			NodeId fromNode;
			SlotIdentifier fromSlot, toSlot;
			SubSignalType fromSub, toSub;
			
			template <typename Archive>
			void serialize(Archive& ar, const uint32 version){
				ar & fromNode;
				ar & fromSlot;
				ar & toSlot;
				ar & fromSub;
				ar & toSub;
			}

			void dump() const {
				print(debug::Ch::Dev, debug::Vb::Trivial, "        fromSlot: %s, toSlot: %s", fromSlot.name.cStr(), toSlot.name.cStr());
			}
		};
		
		struct DefaultValue {
			SlotIdentifier slot;
			boost::any defaultValue;
			
			template <typename Archive>
			void serialize(Archive& ar, const uint32 version){
				ar & slot;
				RuntimeSignalTypeTraits::serializeValue(slot.signalType, ar, version, defaultValue);
			}
		};
		
		struct TemplateSlotWrap {
			util::Str8 groupName;
			bool input; // Is input or output
			SlotIdentifier slotIdentifier;
			
			template <typename Archive>
			void serialize(Archive& ar, const uint32 version){
				ar & groupName;
				ar & input;
				ar & slotIdentifier;
			}
		};
		
		template <typename Archive>
		void serialize(Archive& ar, const uint32 version){
			ar & typeName;
			ar & position;
			ar & inputs;
			ar & defaultValues;
			ar & templateSlots;
		}

		void dump() const {
			print(debug::Ch::Dev, debug::Vb::Trivial, "    typeName: %s", typeName.cStr());
			for (auto& m : inputs)
				m.dump();
		}
		

		
		util::Str8 typeName; // Name-attribute of NodeType
		util::Vec2d position; // Position in editor
		util::DynArray<Input> inputs; // Attached inputs
		util::DynArray<DefaultValue> defaultValues; // Default values for input slots
		util::DynArray<TemplateSlotWrap> templateSlots; // Active dynamic slots
	};
	
	template <typename Archive>
	void serialize(Archive& ar, const uint32 version){
		ar & nodes;
	}

	void dump() const {
		print(debug::Ch::Dev, debug::Vb::Trivial, "CompositionNodeLogicGroup::SerializationGraph dump");
		for (auto& m : nodes)
			m.dump();
	}

	util::DynArray<Node> nodes;
};

void CompositionNodeLogicGroup::loadFromFile(){
	std::ifstream stream(fileAttribute.get().whole().cStr(), std::ios::binary);
	if (!stream)
		throw resources::ResourceException(util::Str8::format("Couldn't open file: %s", fileAttribute.get().whole().cStr()).cStr());
	
	SerializationGraph graph;
	
	try {
		boost::archive::text_iarchive ia(stream);
		ia >> graph;
		
		applySerializationGraph(graph);
	}
	catch (const std::exception& e){
		throw resources::ResourceException(util::Str8::format("Error deserializing a graph for %s: %s", getName().cStr(), e.what()).cStr());
	}
	
}

void CompositionNodeLogicGroup::saveToFile() const {
	std::ofstream stream(fileAttribute.get().whole().cStr(), std::ios::binary);
	if (!stream)
		throw resources::ResourceException(util::Str8::format("Couldn't open file: %s", fileAttribute.get().whole().cStr()).cStr());
	
	try {
		boost::archive::text_oarchive oa(stream);
		const SerializationGraph& graph= generateSerializationGraph();

		oa << graph;

	}
	catch (const std::exception& e){
		throw resources::ResourceException(util::Str8::format("Error serializing a graph for %s: %s", getName().cStr(), e.what()).cStr());
	}
	
}


void CompositionNodeLogicGroup::applySerializationGraph(const SerializationGraph& graph){
	bool error= false;
	
	// Create nodes
	for (auto& graph_node : graph.nodes){
		try {
			add(resources::gCache->getResource<nodes::NodeType>(graph_node.typeName))
				.setPosition(graph_node.position);
		}
		catch (global::Exception& e){
			error= true;
			continue;
		}
		
		// Create template slots
		for (auto& slot_wrap : graph_node.templateSlots){
			nodes.back()->addSlot(
				nodes.back()->getSlotTemplateGroup(slot_wrap.groupName).getSlotTemplate(slot_wrap.slotIdentifier));
		}
		
		// util::Set default values
		for (auto& default_value : graph_node.defaultValues){
			try {
				nodes.back()->getInputSlot(default_value.slot).setDefaultValue(default_value.defaultValue);
			}
			catch (global::Exception& e){
				error= true;
			}
		}
		
	}
	
	// Attach slots
	for (SerializationGraph::NodeId id= 0; id < nodes.size(); ++id){
		for (auto& input : graph.nodes[id].inputs){
			

			/*print(debug::Ch::Dev, debug::Vb::Trivial, "CompositionNode attach: (%s, %s, %s) -> (%s, %s, %s)",
				graph.nodes[input.fromNode].typeName.cStr(),
				input.fromSlot.name.cStr(),
				RuntimeSubSignalTypeTraits::name(input.fromSub).cStr(),
				graph.nodes[id].typeName.cStr(),
				input.toSlot.name.cStr(),
				RuntimeSubSignalTypeTraits::name(input.toSub).cStr()
			);*/
			
			try {
				CompositionNodeSlot& input_slot= nodes[id]->getInputSlot(input.toSlot);
				input_slot.attach(nodes[input.fromNode]->getOutputSlot(input.fromSlot), input.toSub, input.fromSub);
			}
			catch (global::Exception& e){
				error= true;
			}
		}
	}
	
	if (error){
		throw global::Exception("Error deserializing node group");
	}
}

CompositionNodeLogicGroup::SerializationGraph CompositionNodeLogicGroup::generateSerializationGraph() const {
	SerializationGraph graph;
	
	util::Map<const CompositionNodeLogic*, SerializationGraph::NodeId> nodeIdMap;
	
	// Create Nodes
	for (auto& comp_node : nodes){
		nodeIdMap[comp_node.get()]= graph.nodes.size();
		graph.nodes.pushBack(SerializationGraph::Node { comp_node->getType().getName(), comp_node->getPosition() });
		
		// Create default values
		
		for (auto& comp_slot : comp_node->getSlots()){
			if (comp_slot->isInput() && comp_slot->isDefaultValueSet()){
				graph.nodes.back().defaultValues.pushBack(SerializationGraph::Node::DefaultValue {
					comp_slot->getIdentifier(),
					comp_slot->getDefaultValue()
				});
			}
			
		}
		
		for (auto& comp_slot : comp_node->getSlots()){
			// Template slots
			if (comp_slot->getTemplateGroup()){
					
				graph.nodes.back().templateSlots.pushBack(SerializationGraph::Node::TemplateSlotWrap{
					comp_slot->getTemplateGroup()->getName(),
					comp_slot->getTemplateGroup()->isInput(),
					comp_slot->getIdentifier()
				});
				
			}
		}
	
	}
	
	// Create slot connections
	for (auto& comp_node : nodes){
		util::DynArray<CompositionNodeSlot*> comp_slots= comp_node->getSlots();
		
		SerializationGraph::Node& node= graph.nodes[nodeIdMap[comp_node.get()]];
		
		for (auto& comp_slot : comp_slots){
			if (!comp_slot->isInput()) continue;
			
			const util::DynArray<CompositionNodeSlot::AttachedSlotInfo>& attached_infos= comp_slot->getAttachedSlotInfos();
			
			for (auto& info : attached_infos){
				node.inputs.pushBack(SerializationGraph::Node::Input {
					nodeIdMap[&info.slot->getOwner()],
					info.slot->getIdentifier(),
					comp_slot->getIdentifier(),
					info.slotSub,
					info.mySub
				});
			}
			
		}
	}
	
	
	return (graph);
}

} // nodes
} // clover