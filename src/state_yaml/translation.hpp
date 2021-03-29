#ifndef TRANSLATION_HPP
#define TRANSLATION_HPP

#include <foe/ecs/entity_id.hpp>

struct GroupTranslation {
    foeGroupID source;
    foeGroupID target;
};

#endif // TRANSLATION_HPP