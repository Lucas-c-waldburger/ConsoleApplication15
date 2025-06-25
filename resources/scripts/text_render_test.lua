-- transform
-- render_profile
-- text_renderable

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

text_renderable.text = "Dude he fucking\nturns himself into a pickle.\nFunniest shit I've ever seen"
text_renderable.dimensions.w = 700
text_renderable.dimensions.h = 250
text_renderable.align = TextAlign.Center
text_renderable.dirtyFlags = DirtyFlag.NewText

