#include "Entity.h"
#include "../Rendering/Renderer.h"

namespace Engine {

void Entity::render(Renderer& renderer) {
    if (texture)
        renderer.drawSprite(position, size, *texture);
    else
        renderer.drawRectangle(position, size, color);
}

}
