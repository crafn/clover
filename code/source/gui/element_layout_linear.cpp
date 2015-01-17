#include "element_layout_linear.hpp"
#include "hardware/mouse.hpp"
#include "debug/draw.hpp"

/// Too lazy to translate this file, but the current
/// gui system is going to trash for most parts anyway

namespace clover {
namespace gui {

LinearLayoutElement::LinearLayoutElement(Type t, const util::Coord& offset, const util::Coord& rad):
						LayoutElement(offset, rad),
						type(t),
						centerSecondary(false),
						margin(true),
						firstNodeSideMirrored(false),
						automaticMinRadius(true),
						ignoreInactive(false){
	elementType= LinearLayout;
	minSpacing.setType(util::Coord::View_Fit);
	minSpacing.setRelative();

	maxSpacing.setType(util::Coord::View_Fit);
	maxSpacing= util::Coord::VF(10000000);
	maxSpacing.setRelative();
		
	stretchable= true;
	
	// Radius joka annetaan parametrina tulkitaan maksimiradiukseksi
	uint32 component= 1;
	if (type == Vertical) component= 0;
	//minRadius[component]= 0; //minRadius limitataan elementeistä riippuviin minimeihin joka frame
	//maxRadius= rad;
	//maxRadius.setRelative();
}


LinearLayoutElement::~LinearLayoutElement(){
	
}

void LinearLayoutElement::setMinSpacing(const util::Coord& min_spacing){
	minSpacing.setType(min_spacing.getType());
	minSpacing %= min_spacing;
}

void LinearLayoutElement::setMaxSpacing(const util::Coord& max_spacing){
	maxSpacing.setType(max_spacing.getType());
	maxSpacing %= max_spacing;
}

void LinearLayoutElement::minimizeRadius(){
	util::Coord min= calcMinimumRadius();
	
	if (minRadius.x > min.x)
		min.x= minRadius.x;
	
	if (minRadius.y > min.y)
		min.y= minRadius.y;
	
	setMaxRadius(min);
	setMinRadius(min);
	setRadius(min);
	
}

void LinearLayoutElement::preUpdate(){
	LayoutElement::preUpdate();
}

util::Coord LinearLayoutElement::getSecLimitedRadius(Element& e){
	
	uint32 sec_component= 1;
	if (type == Vertical) sec_component= 0;
	
	
	// Limitoidaan radiukset, että ne pysyvät sekundäärisen komponentin sisässä
	{
		auto radius_type= radius.getType();
		real64 rad_from_center= util::abs(e.getRadius().converted(radius_type)[sec_component])
				+ util::abs(e.getOffset().converted(radius_type)[sec_component]);
		if (radius[sec_component] < rad_from_center){ // Jos radius menee yli reunoilta
			
			real64 over= rad_from_center-radius[sec_component];
			real64 rad= e.getRadius().converted(radius_type)[sec_component];
			util::Coord min_rad= e.getMinRadius().converted(radius_type);
			util::Coord max_rad= e.getMaxRadius().converted(radius_type);
			// Ettei jaeta nollalla
			if (min_rad[sec_component] == max_rad[sec_component]) return e.getMaxRadius();
			
			real64 target= rad - over;
			
			// mul = 0, kun target=max ja 1 kun target=min

			real64 mul= util::limited((target-max_rad[sec_component])/(min_rad[sec_component]-max_rad[sec_component]), 0.0, 1.0);
			
			//Asetetaan pienempi radius
			return (min_rad*mul + max_rad*(1-mul));
			
		}
	}
	return e.getMaxRadius();
}

util::Coord LinearLayoutElement::calcMinimumRadius() const {
	uint32 component= 0;
	uint32 sec_component= 1;
	
	if (type == Vertical){
		component= 1;
		sec_component= 0;
	}
	
	util::Coord min_radius(0.0, minRadius.getType());
	
	// Minimi componentin ja sec_componentin suunnassa (min spacing ei vaikuta, pitäisikö?)
	for (auto &m : nodes){
		if (ignoreInactive && !m.getElement().isActive()) continue;
		
		const Element& e= m.getElement();
		util::Vec2d cur_min_radius= e.getMinRadius().converted(minRadius.getType()).getValue();
		min_radius[component] += cur_min_radius[component];
		
		real64 rad= cur_min_radius[sec_component] + util::abs(e.getOffset().converted(minRadius.getType())[sec_component]);
		if (rad > min_radius[sec_component]){
			min_radius[sec_component]= rad;
		}
	}
	
	return min_radius;
}

void LinearLayoutElement::updateNodes(){
	uint32 component= 0;
	if (type == Vertical) component= 1;
	
	uint32 sec_component= 1;
	if (type == Vertical) sec_component= 0;
	
	if (nodes.empty()){
		//radius[component]= minRadius[component];
		return;
	}
	
	// Järjetellään nodet niiden positioneiden mukaan
	std::sort(nodes.begin(), nodes.end());
	
	auto radius_type= radius.getType();
	
	real64 preferred_min_spacing= minSpacing.converted(radius_type)[component];
	real64 preferred_max_spacing= maxSpacing.converted(radius_type)[component];
	
	// Paljonko ylimääränen väli tuo lisää elementtien yhteiseen radiukseen
	real64 spacing_rad_addition= preferred_min_spacing * (nodes.size() - 1)*0.5;
	real64 spacing_max_rad_sum= preferred_max_spacing * (nodes.size() - 1)*0.5;
	
	// Lasketaan prosentti, että miten pitää skaalata elementtejä, että ne mahtuu
	real64 min_rad_sum= 0;
	real64 max_rad_sum= 0;
	
	uint32 i=0;
	
	for (auto &m : nodes){
		if (ignoreInactive && !m.getElement().isActive()) continue;
		
		Element& e= m.getElement();

		min_rad_sum += e.getMinRadius().converted(radius_type)[component];
		max_rad_sum += e.getMaxRadius().converted(radius_type)[component];
		
		++i;
	}
	
	real64 h0= min_rad_sum - radius[component];
	real64 h1= max_rad_sum - radius[component];
	
	real64 empty_space=0; // Kokonaismäärä tyhjää
	
	real64 percentage= 0; // Kuvaa kuinka elementtejä pitää skaalata: 0: Minimiradius, 1: Maksimiradius
	if ( util::abs(h0 - h1) >= util::epsilon && util::abs(max_rad_sum - min_rad_sum) >= util::epsilon ){
		percentage= 1-h0/(h0-h1); // Tämä pätee, jos spacing = 0
		
		/* Alla oleva johdetaan yhtälöistä: (p eli percentage on radiuksen parametri)
		 *		p*min_rad_sum + (1-p)*max_rad_sum = radius(p)
		 *		spacing_rad_addition + radius(p+a)= radius(p)
		 *		radius(p) = vakio
		 */

		real64 a= spacing_rad_addition/(max_rad_sum-min_rad_sum);
		percentage += a;
		percentage= util::limited(percentage, 0.0, 1.0);
	}
	
	real64 element_rad_sum= (min_rad_sum*percentage + max_rad_sum*(1-percentage));
	
	if (empty_space == 0)
		// *2 niin saadaan radiuksista halkaisijoita
		empty_space= (radius[component] - element_rad_sum)*2; 
	
	// Kokonaismäärä tyhjää elementtien välissä (ennen maksimien tai minimien aiheuttamia rajoituksia)
	real64 empty_space_between_elements= empty_space;

	// Ilman marginaalia sama kuin empty_space
	if (margin){
		empty_space_between_elements= empty_space/(nodes.size()+1)*(nodes.size()-1);
	}
	
	ensure(std::isfinite(empty_space_between_elements));
	
	real64 dist=0; // Etäisyys yläreunasta / vasemmasta reunasta
	
	if (margin){ // Jos pitäs olla marginaali
		
		// Meinaavatko jäädä kauemmas toisistaan kuin maksimi sallittu
		if (empty_space_between_elements >= spacing_max_rad_sum*2){
			
			// Otetaan puolet ylijäävästä tyhjästä tilasta ja lisätään ekan noden paikkaan, niin keskittyy (= margin)
			dist += (empty_space - spacing_max_rad_sum*2)*0.5;
			
			// Väliä jätetään maksimiväli
			empty_space= preferred_max_spacing;
			
		}
		else {
			// Eivät jää liian kauas

			if (empty_space / ((real64)nodes.size() + 1) >= preferred_min_spacing){ 
				// Mahtuu olemaan min_spacingin päässä toisistaan
				dist += (empty_space / ((real64)nodes.size() + 1));
				empty_space /= (real64)nodes.size() + 1;
			}
			else {
				// Ei mahu
				ensure(nodes.size() - 1);
				empty_space /= (real64)nodes.size() - 1;
			}
		}
	}
	else {
		// Ei jätetä tilaa reunoille
		
		// Meinaavatko jäädä kauemmas toisistaan kuin maksimi sallittu
		if (empty_space_between_elements >= spacing_max_rad_sum*2){	
			if (firstNodeSideMirrored){
				// Siirretään nodet toiselle puolelle
				dist += empty_space - preferred_max_spacing*((real64)nodes.size()-1);
			}
			empty_space= preferred_max_spacing;
		}
		else {
			// Nodet alle maksimietäisyyden päässä toisistaan
			
			// Paljonko jokaisen noden välissä on tyhjää
			if (nodes.size() > 1)
				empty_space /= (real64)nodes.size() - 1;
			else if (nodes.size() == 1 && firstNodeSideMirrored){
				dist += empty_space;
			}
		}
	}

	//print(debug::Ch::Gui, debug::Vb::Trivial, "asd %f", preferred_min_spacing);
	// Säädetään oikeille paikoille oikeilla säteillä
	
	ensure(std::isfinite(empty_space));
	ensure(std::isfinite(radius.x) && std::isfinite(radius.y));
	ensure(std::isfinite(percentage));
	
	int32 layout_dir_mul= 1;
	if (type == Vertical) layout_dir_mul= -1; // From top to bottom
	
	for (auto& m : nodes){
		if (ignoreInactive && !m.getElement().isActive()) continue;
		gui::Element& e= m.getElement();
		
		util::Coord target_offset= e.getOffset();
		target_offset.convertTo(radius_type);
		
		util::Coord min_rad= e.getMinRadius().converted(radius_type);
		util::Coord max_rad= e.getMaxRadius().converted(radius_type);
		
		ensure(!std::isnan(target_offset.x) && !std::isnan(target_offset.y));
		ensure(std::isfinite(min_rad.x) && std::isfinite(min_rad.y));
		ensure(std::isfinite(max_rad.x) && std::isfinite(max_rad.y));
		
		if (min_rad.x - max_rad.x > util::epsilon || min_rad.y - max_rad.y > util::epsilon){
			print(debug::Ch::Gui, debug::Vb::Moderate, "Min radius bigger than max %i: min %f, %f max: %f, %f", e.getType(), min_rad.x, min_rad.y, max_rad.x, max_rad.y);
			global::g_env.debugDraw->addFilledRect(e.getPosition(), e.getRadius(), util::Color{1.0,0,0,0.5}, 0);
			global::g_env.debugDraw->addCross(e.getPosition(), util::Color{1.0,0,0,0.3});
		}

		util::Coord target_rad= (min_rad)*percentage + max_rad*(1-percentage);
		
		if (m.isStretchable()){
			// Stretch secondary
			target_rad[sec_component]= util::limited(radius[sec_component], min_rad[sec_component], max_rad[sec_component]);
		}

		dist += target_rad[component];
		
		// komponentti menee layoutin määräämänä
		target_offset[component]= util::limited(	(-radius[component] + dist)*layout_dir_mul, 
											-radius[component] + target_rad[component],
											radius[component] - target_rad[component]); // Limitoidaan vielä ettei mene yli rajoista
		
		dist += target_rad[component] + empty_space;
		
		ensure(std::isfinite(target_offset.x) && std::isfinite(target_offset.y));
		//if (component == 0)
		//	target_offset.y= e.getOffset().y;
		
		if (centerSecondary){
			if (component == 0)
				target_offset.y= 0;
			else
				target_offset.x= 0;
		}
		
		e.setOffset(target_offset);
		
		ensure(std::isfinite(target_rad.x) && std::isfinite(target_rad.y));
		e.setRadius(target_rad.converted(e.getRadius().getType()));
		
		util::Coord rad_limited_by_sec= getSecLimitedRadius(e).converted(radius_type);
		if (rad_limited_by_sec[component] < target_rad[component]){
			ensure(std::isfinite(rad_limited_by_sec.x) && std::isfinite(rad_limited_by_sec.y));
			e.setRadius(rad_limited_by_sec.converted(e.getRadius().getType()));
		}
		
	}
}

void LinearLayoutElement::spatialUpdate(){
	BaseType::spatialUpdate();
	
	if (isActive()){
		if (nodes.empty() ){
			global::g_env.debugDraw->addText(position, "Empty LinearLayout", util::Vec2d(0.5), util::Color{0, 0, 1, 0.5});
		}
		else {
			global::g_env.debugDraw->addText(position, "LinearLayout", util::Vec2d(0.5), util::Color{0, 0, 1, 0.5});
		}
	}
	
	uint32 sec_component= 1;
	if (type == Vertical) sec_component= 0;
	
	uint32 component=0;
	if (sec_component == 0) component=1;
	
	if (superElement){ // asetetaan sekundäärinen komponentti parentin kokoseks
		radius[sec_component]= util::limited(superElement->getRadius().converted(radius.getType())[sec_component],
											 minRadius.converted(radius.getType())[sec_component],
											 maxRadius.converted(radius.getType())[sec_component]);
	}

	if (automaticMinRadius){
		util::Coord min_radius= calcMinimumRadius().converted(minRadius.getType());
		setMinRadius(min_radius);
	}

	updateNodes();
}

} // gui
} // clover
