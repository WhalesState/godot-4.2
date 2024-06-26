/**************************************************************************/
/*  placeholder_textures.cpp                                              */
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

#include "placeholder_textures.h"

void PlaceholderTexture2D::set_size(Size2 p_size) {
	size = p_size;
	emit_changed();
}

int PlaceholderTexture2D::get_width() const {
	return size.width;
}

int PlaceholderTexture2D::get_height() const {
	return size.height;
}

bool PlaceholderTexture2D::has_alpha() const {
	return false;
}

Ref<Image> PlaceholderTexture2D::get_image() const {
	return Ref<Image>();
}

RID PlaceholderTexture2D::get_rid() const {
	if (rid.is_null()) {
		rid = RenderingServer::get_singleton()->texture_2d_placeholder_create();
	}
	return rid;
}

void PlaceholderTexture2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_size", "size"), &PlaceholderTexture2D::set_size);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size", PROPERTY_HINT_NONE, "suffix:px"), "set_size", "get_size");
}

PlaceholderTexture2D::PlaceholderTexture2D() {
}

PlaceholderTexture2D::~PlaceholderTexture2D() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	if (rid.is_valid()) {
		RS::get_singleton()->free(rid);
	}
}
