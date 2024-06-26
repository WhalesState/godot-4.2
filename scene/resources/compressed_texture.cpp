/**************************************************************************/
/*  compressed_texture.cpp                                                */
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

#include "compressed_texture.h"

#include "scene/resources/bit_map.h"

Error CompressedTexture2D::_load_data(const String &p_path, int &r_width, int &r_height, Ref<Image> &image, bool &r_request_3d, bool &r_request_normal, bool &r_request_roughness, int &mipmap_limit, int p_size_limit) {
	alpha_cache.unref();

	ERR_FAIL_COND_V(image.is_null(), ERR_INVALID_PARAMETER);

	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(f.is_null(), ERR_CANT_OPEN, vformat("Unable to open file: %s.", p_path));

	uint8_t header[4];
	f->get_buffer(header, 4);
	if (header[0] != 'G' || header[1] != 'S' || header[2] != 'T' || header[3] != '2') {
		ERR_FAIL_V_MSG(ERR_FILE_CORRUPT, "Compressed texture file is corrupt (Bad header).");
	}

	uint32_t version = f->get_32();

	if (version > FORMAT_VERSION) {
		ERR_FAIL_V_MSG(ERR_FILE_CORRUPT, "Compressed texture file is too new.");
	}
	r_width = f->get_32();
	r_height = f->get_32();
	uint32_t df = f->get_32(); //data format

	//skip reserved
	mipmap_limit = int(f->get_32());
	//reserved
	f->get_32();
	f->get_32();
	f->get_32();

#ifdef TOOLS_ENABLED

	r_request_3d = request_3d_callback && df & FORMAT_BIT_DETECT_3D;
	r_request_roughness = request_roughness_callback && df & FORMAT_BIT_DETECT_ROUGNESS;
	r_request_normal = request_normal_callback && df & FORMAT_BIT_DETECT_NORMAL;

#else

	r_request_3d = false;
	r_request_roughness = false;
	r_request_normal = false;

#endif
	if (!(df & FORMAT_BIT_STREAM)) {
		p_size_limit = 0;
	}

	image = load_image_from_file(f, p_size_limit);

	if (image.is_null() || image->is_empty()) {
		return ERR_CANT_OPEN;
	}

	return OK;
}

void CompressedTexture2D::set_path(const String &p_path, bool p_take_over) {
	if (texture.is_valid()) {
		RenderingServer::get_singleton()->texture_set_path(texture, p_path);
	}

	Resource::set_path(p_path, p_take_over);
}

void CompressedTexture2D::_requested_3d(void *p_ud) {
	CompressedTexture2D *ct = (CompressedTexture2D *)p_ud;
	Ref<CompressedTexture2D> ctex(ct);
	ERR_FAIL_NULL(request_3d_callback);
	request_3d_callback(ctex);
}

void CompressedTexture2D::_requested_roughness(void *p_ud, const String &p_normal_path, RS::TextureDetectRoughnessChannel p_roughness_channel) {
	CompressedTexture2D *ct = (CompressedTexture2D *)p_ud;
	Ref<CompressedTexture2D> ctex(ct);
	ERR_FAIL_NULL(request_roughness_callback);
	request_roughness_callback(ctex, p_normal_path, p_roughness_channel);
}

void CompressedTexture2D::_requested_normal(void *p_ud) {
	CompressedTexture2D *ct = (CompressedTexture2D *)p_ud;
	Ref<CompressedTexture2D> ctex(ct);
	ERR_FAIL_NULL(request_normal_callback);
	request_normal_callback(ctex);
}

CompressedTexture2D::TextureFormatRequestCallback CompressedTexture2D::request_3d_callback = nullptr;
CompressedTexture2D::TextureFormatRoughnessRequestCallback CompressedTexture2D::request_roughness_callback = nullptr;
CompressedTexture2D::TextureFormatRequestCallback CompressedTexture2D::request_normal_callback = nullptr;

Image::Format CompressedTexture2D::get_format() const {
	return format;
}

Error CompressedTexture2D::load(const String &p_path) {
	int lw, lh;
	Ref<Image> image;
	image.instantiate();

	bool request_3d;
	bool request_normal;
	bool request_roughness;
	int mipmap_limit;

	Error err = _load_data(p_path, lw, lh, image, request_3d, request_normal, request_roughness, mipmap_limit);
	if (err) {
		return err;
	}

	if (texture.is_valid()) {
		RID new_texture = RS::get_singleton()->texture_2d_create(image);
		RS::get_singleton()->texture_replace(texture, new_texture);
	} else {
		texture = RS::get_singleton()->texture_2d_create(image);
	}
	if (lw || lh) {
		RS::get_singleton()->texture_set_size_override(texture, lw, lh);
	}

	w = lw;
	h = lh;
	path_to_file = p_path;
	format = image->get_format();

	if (get_path().is_empty()) {
		//temporarily set path if no path set for resource, helps find errors
		RenderingServer::get_singleton()->texture_set_path(texture, p_path);
	}

#ifdef TOOLS_ENABLED

	if (request_3d) {
		//print_line("request detect 3D at " + p_path);
		RS::get_singleton()->texture_set_detect_3d_callback(texture, _requested_3d, this);
	} else {
		//print_line("not requesting detect 3D at " + p_path);
		RS::get_singleton()->texture_set_detect_3d_callback(texture, nullptr, nullptr);
	}

	if (request_roughness) {
		//print_line("request detect srgb at " + p_path);
		RS::get_singleton()->texture_set_detect_roughness_callback(texture, _requested_roughness, this);
	} else {
		//print_line("not requesting detect srgb at " + p_path);
		RS::get_singleton()->texture_set_detect_roughness_callback(texture, nullptr, nullptr);
	}

	if (request_normal) {
		//print_line("request detect srgb at " + p_path);
		RS::get_singleton()->texture_set_detect_normal_callback(texture, _requested_normal, this);
	} else {
		//print_line("not requesting detect normal at " + p_path);
		RS::get_singleton()->texture_set_detect_normal_callback(texture, nullptr, nullptr);
	}

#endif
	notify_property_list_changed();
	emit_changed();
	return OK;
}

String CompressedTexture2D::get_load_path() const {
	return path_to_file;
}

int CompressedTexture2D::get_width() const {
	return w;
}

int CompressedTexture2D::get_height() const {
	return h;
}

RID CompressedTexture2D::get_rid() const {
	if (!texture.is_valid()) {
		texture = RS::get_singleton()->texture_2d_placeholder_create();
	}
	return texture;
}

void CompressedTexture2D::draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate, bool p_transpose) const {
	if ((w | h) == 0) {
		return;
	}
	RenderingServer::get_singleton()->canvas_item_add_texture_rect(p_canvas_item, Rect2(p_pos, Size2(w, h)), texture, false, p_modulate, p_transpose);
}

void CompressedTexture2D::draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile, const Color &p_modulate, bool p_transpose) const {
	if ((w | h) == 0) {
		return;
	}
	RenderingServer::get_singleton()->canvas_item_add_texture_rect(p_canvas_item, p_rect, texture, p_tile, p_modulate, p_transpose);
}

void CompressedTexture2D::draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate, bool p_transpose, bool p_clip_uv) const {
	if ((w | h) == 0) {
		return;
	}
	RenderingServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, p_rect, texture, p_src_rect, p_modulate, p_transpose, p_clip_uv);
}

bool CompressedTexture2D::has_alpha() const {
	return false;
}

Ref<Image> CompressedTexture2D::get_image() const {
	if (texture.is_valid()) {
		return RS::get_singleton()->texture_2d_get(texture);
	} else {
		return Ref<Image>();
	}
}

bool CompressedTexture2D::is_pixel_opaque(int p_x, int p_y) const {
	if (!alpha_cache.is_valid()) {
		Ref<Image> img = get_image();
		if (img.is_valid()) {
			if (img->is_compressed()) { //must decompress, if compressed
				Ref<Image> decom = img->duplicate();
				decom->decompress();
				img = decom;
			}

			alpha_cache.instantiate();
			alpha_cache->create_from_image_alpha(img);
		}
	}

	if (alpha_cache.is_valid()) {
		int aw = int(alpha_cache->get_size().width);
		int ah = int(alpha_cache->get_size().height);
		if (aw == 0 || ah == 0) {
			return true;
		}

		int x = p_x * aw / w;
		int y = p_y * ah / h;

		x = CLAMP(x, 0, aw);
		y = CLAMP(y, 0, ah);

		return alpha_cache->get_bit(x, y);
	}

	return true;
}

void CompressedTexture2D::reload_from_file() {
	String path = get_path();
	if (!path.is_resource_file()) {
		return;
	}

	path = ResourceLoader::path_remap(path); //remap for translation
	path = ResourceLoader::import_remap(path); //remap for import
	if (!path.is_resource_file()) {
		return;
	}

	load(path);
}

void CompressedTexture2D::_validate_property(PropertyInfo &p_property) const {
}

Ref<Image> CompressedTexture2D::load_image_from_file(Ref<FileAccess> f, int p_size_limit) {
	uint32_t data_format = f->get_32();
	uint32_t w = f->get_16();
	uint32_t h = f->get_16();
	uint32_t mipmaps = f->get_32();
	Image::Format format = Image::Format(f->get_32());

	if (data_format == DATA_FORMAT_PNG || data_format == DATA_FORMAT_WEBP) {
		//look for a PNG or WebP file inside

		int sw = w;
		int sh = h;

		//mipmaps need to be read independently, they will be later combined
		Vector<Ref<Image>> mipmap_images;
		uint64_t total_size = 0;

		bool first = true;

		for (uint32_t i = 0; i < mipmaps + 1; i++) {
			uint32_t size = f->get_32();

			if (p_size_limit > 0 && i < (mipmaps - 1) && (sw > p_size_limit || sh > p_size_limit)) {
				//can't load this due to size limit
				sw = MAX(sw >> 1, 1);
				sh = MAX(sh >> 1, 1);
				f->seek(f->get_position() + size);
				continue;
			}

			Vector<uint8_t> pv;
			pv.resize(size);
			{
				uint8_t *wr = pv.ptrw();
				f->get_buffer(wr, size);
			}

			Ref<Image> img;
			if (data_format == DATA_FORMAT_PNG && Image::png_unpacker) {
				img = Image::png_unpacker(pv);
			} else if (data_format == DATA_FORMAT_WEBP && Image::webp_unpacker) {
				img = Image::webp_unpacker(pv);
			}

			if (img.is_null() || img->is_empty()) {
				ERR_FAIL_COND_V(img.is_null() || img->is_empty(), Ref<Image>());
			}

			if (first) {
				//format will actually be the format of the first image,
				//as it may have changed on compression
				format = img->get_format();
				first = false;
			} else if (img->get_format() != format) {
				img->convert(format); //all needs to be the same format
			}

			total_size += img->get_data().size();

			mipmap_images.push_back(img);

			sw = MAX(sw >> 1, 1);
			sh = MAX(sh >> 1, 1);
		}

		//print_line("mipmap read total: " + itos(mipmap_images.size()));

		Ref<Image> image;
		image.instantiate();

		if (mipmap_images.size() == 1) {
			//only one image (which will most likely be the case anyway for this format)
			image = mipmap_images[0];
			return image;

		} else {
			//rarer use case, but needs to be supported
			Vector<uint8_t> img_data;
			img_data.resize(total_size);

			{
				uint8_t *wr = img_data.ptrw();

				int ofs = 0;
				for (int i = 0; i < mipmap_images.size(); i++) {
					Vector<uint8_t> id = mipmap_images[i]->get_data();
					int len = id.size();
					const uint8_t *r = id.ptr();
					memcpy(&wr[ofs], r, len);
					ofs += len;
				}
			}

			image->set_data(w, h, true, mipmap_images[0]->get_format(), img_data);
			return image;
		}

	} else if (data_format == DATA_FORMAT_IMAGE) {
		int size = Image::get_image_data_size(w, h, format, mipmaps ? true : false);

		for (uint32_t i = 0; i < mipmaps + 1; i++) {
			int tw, th;
			int ofs = Image::get_image_mipmap_offset_and_dimensions(w, h, format, i, tw, th);

			if (p_size_limit > 0 && i < mipmaps && (p_size_limit > tw || p_size_limit > th)) {
				if (ofs) {
					f->seek(f->get_position() + ofs);
				}
				continue; //oops, size limit enforced, go to next
			}

			Vector<uint8_t> data;
			data.resize(size - ofs);

			{
				uint8_t *wr = data.ptrw();
				f->get_buffer(wr, data.size());
			}

			Ref<Image> image = Image::create_from_data(tw, th, mipmaps - i ? true : false, format, data);

			return image;
		}
	}

	return Ref<Image>();
}

void CompressedTexture2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load", "path"), &CompressedTexture2D::load);
	ClassDB::bind_method(D_METHOD("get_load_path"), &CompressedTexture2D::get_load_path);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "load_path", PROPERTY_HINT_FILE, "*.ctex"), "load", "get_load_path");
}

CompressedTexture2D::CompressedTexture2D() {}

CompressedTexture2D::~CompressedTexture2D() {
	if (texture.is_valid()) {
		ERR_FAIL_NULL(RenderingServer::get_singleton());
		RS::get_singleton()->free(texture);
	}
}

Ref<Resource> ResourceFormatLoaderCompressedTexture2D::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	Ref<CompressedTexture2D> st;
	st.instantiate();
	Error err = st->load(p_path);
	if (r_error) {
		*r_error = err;
	}
	if (err != OK) {
		return Ref<Resource>();
	}

	return st;
}

void ResourceFormatLoaderCompressedTexture2D::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("ctex");
}

bool ResourceFormatLoaderCompressedTexture2D::handles_type(const String &p_type) const {
	return p_type == "CompressedTexture2D";
}

String ResourceFormatLoaderCompressedTexture2D::get_resource_type(const String &p_path) const {
	if (p_path.get_extension().to_lower() == "ctex") {
		return "CompressedTexture2D";
	}
	return "";
}
