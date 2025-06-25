-- transform
-- render_profile
-- sprite_renderable
-- sprite_animations

transform.position.x = 400.0
transform.position.y = 400.0
transform.rotation = 0.0
transform.scale.x = 1.0
transform.scale.y = 1.0

render_profile.drawOrder = 0
render_profile.flip = SDL_RendererFlip.SDL_FLIP_NONE
render_profile.offset.x = 0.0
render_profile.offset.y = 0.0
render_profile.debugDraw.boundingBox.on = true
render_profile.debugDraw.boundingBox.color.r = 150
render_profile.debugDraw.boundingBox.color.g = 150
render_profile.debugDraw.boundingBox.color.b = 200

sprite_animations.dirty = true
sprite_animations.map:Next()