#ifndef CLOVER_PHYSICS_GHOSTLYDYNAMICS_HPP
#define CLOVER_PHYSICS_GHOSTLYDYNAMICS_HPP

#include "build.hpp"
#include "contact.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace physics {

bool isGhostlyContact(const Contact& c);
PostSolveContact onGhostlyContact(const Contact& c);
void processGhostlyInteractions(const util::DynArray<Contact>& contacts, real64 dt);

} // physics
} // clover

#endif // CLOVER_PHYSICS_GHOSTLYDYNAMICS_HPP
