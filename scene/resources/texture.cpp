/**************************************************************************/
/*  texture.cpp                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "texture.h"

#include "scene/resources/placeholder_textures.h"

int Texture2D::get_width() const {
	int ret = 0;
	GDVIRTUAL_REQUIRED_CALL(_get_width, ret);
	return ret;
}

int Texture2D::get_height() const {
	int ret = 0;
	GDVIRTUAL_REQUIRED_CALL(_get_height, ret);
	return ret;
}

Size2 Texture2D::get_size() const {
	return Size2(get_width(), get_height());
}

bool Texture2D::is_pixel_opaque(int p_x, int p_y) const {
	bool ret = true;
	GDVIRTUAL_CALL(_is_pixel_opaque, p_x, p_y, ret);
	return ret;
}

bool Texture2D::has_alpha() const {
	bool ret = true;
	GDVIRTUAL_CALL(_has_alpha, ret);
	return ret;
}

void Texture2D::draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate, bool p_transpose) const {
	if (GDVIRTUAL_CALL(_draw, p_canvas_item, p_pos, p_modulate, p_transpose)) {
		return;
	}
	RenderingServer::get_singleton()->canvas_item_add_texture_rect(p_canvas_item, Rect2(p_pos, get_size()), get_rid(), false, p_modulate, p_transpose);
}

void Texture2D::draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile, const Color &p_modulate, bool p_transpose) const {
	if (GDVIRTUAL_CALL(_draw_rect, p_canvas_item, p_rect, p_tile, p_modulate, p_transpose)) {
		return;
	}
	RenderingServer::get_singleton()->canvas_item_add_texture_rect(p_canvas_item, p_rect, get_rid(), p_tile, p_modulate, p_transpose);
}

void Texture2D::draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate, bool p_transpose, bool p_clip_uv) const {
	if (GDVIRTUAL_CALL(_draw_rect_region, p_canvas_item, p_rect, p_src_rect, p_modulate, p_transpose, p_clip_uv)) {
		return;
	}
	RenderingServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, p_rect, get_rid(), p_src_rect, p_modulate, p_transpose, p_clip_uv);
}

bool Texture2D::get_rect_region(const Rect2 &p_rect, const Rect2 &p_src_rect, Rect2 &r_rect, Rect2 &r_src_rect) const {
	r_rect = p_rect;
	r_src_rect = p_src_rect;
	return true;
}

Ref<Resource> Texture2D::create_placeholder() const {
	Ref<PlaceholderTexture2D> placeholder;
	placeholder.instantiate();
	placeholder->set_size(get_size());
	return placeholder;
}

void Texture2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_width"), &Texture2D::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &Texture2D::get_height);
	ClassDB::bind_method(D_METHOD("get_size"), &Texture2D::get_size);
	ClassDB::bind_method(D_METHOD("has_alpha"), &Texture2D::has_alpha);
	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "position", "modulate", "transpose"), &Texture2D::draw, DEFVAL(Color(1, 1, 1)), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("draw_rect", "canvas_item", "rect", "tile", "modulate", "transpose"), &Texture2D::draw_rect, DEFVAL(Color(1, 1, 1)), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("draw_rect_region", "canvas_item", "rect", "src_rect", "modulate", "transpose", "clip_uv"), &Texture2D::draw_rect_region, DEFVAL(Color(1, 1, 1)), DEFVAL(false), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("get_image"), &Texture2D::get_image);
	ClassDB::bind_method(D_METHOD("create_placeholder"), &Texture2D::create_placeholder);

	ADD_GROUP("", "");

	GDVIRTUAL_BIND(_get_width);
	GDVIRTUAL_BIND(_get_height);
	GDVIRTUAL_BIND(_is_pixel_opaque, "x", "y");
	GDVIRTUAL_BIND(_has_alpha);

	GDVIRTUAL_BIND(_draw, "to_canvas_item", "pos", "modulate", "transpose")
	GDVIRTUAL_BIND(_draw_rect, "to_canvas_item", "rect", "tile", "modulate", "transpose")
	GDVIRTUAL_BIND(_draw_rect_region, "to_canvas_item", "rect", "src_rect", "modulate", "transpose", "clip_uv");
}

Texture2D::Texture2D() {
}
