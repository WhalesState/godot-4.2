/**************************************************************************/
/*  rendering_server.h                                                    */
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

#ifndef RENDERING_SERVER_H
#define RENDERING_SERVER_H

#include "core/io/image.h"
#include "core/math/transform_2d.h"
#include "core/object/class_db.h"
#include "core/templates/rid.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "servers/display_server.h"

template <typename T>
class TypedArray;

class RenderingServer : public Object {
	GDCLASS(RenderingServer, Object);

	static RenderingServer *singleton;

	int mm_policy = 0;
	bool render_loop_enabled = true;

	const Vector2 SMALL_VEC2 = Vector2(CMP_EPSILON, CMP_EPSILON);
	const Vector3 SMALL_VEC3 = Vector3(CMP_EPSILON, CMP_EPSILON, CMP_EPSILON);

	virtual TypedArray<StringName> _global_shader_parameter_get_list() const;

protected:
	void _free_internal_rids();
	RID test_texture;
	RID white_texture;
	RID test_material;

	static RenderingServer *(*create_func)();
	static void _bind_methods();

public:
	static RenderingServer *get_singleton();
	static RenderingServer *create();

	enum {
		NO_INDEX_ARRAY = -1,
		CANVAS_ITEM_Z_MIN = -4096,
		CANVAS_ITEM_Z_MAX = 4096,
		MAX_GLOW_LEVELS = 7,
		MAX_CURSORS = 8,
		MAX_2D_DIRECTIONAL_LIGHTS = 8,
		MAX_MESH_SURFACES = 256
	};

	/* TEXTURE API */

	virtual RID texture_2d_create(const Ref<Image> &p_image) = 0;
	virtual RID texture_proxy_create(RID p_base) = 0;

	virtual void texture_2d_update(RID p_texture, const Ref<Image> &p_image, int p_layer = 0) = 0;
	virtual void texture_proxy_update(RID p_texture, RID p_proxy_to) = 0;

	// These two APIs can be used together or in combination with the others.
	virtual RID texture_2d_placeholder_create() = 0;

	virtual Ref<Image> texture_2d_get(RID p_texture) const = 0;

	virtual void texture_replace(RID p_texture, RID p_by_texture) = 0;
	virtual void texture_set_size_override(RID p_texture, int p_width, int p_height) = 0;

	virtual void texture_set_path(RID p_texture, const String &p_path) = 0;
	virtual String texture_get_path(RID p_texture) const = 0;

	virtual Image::Format texture_get_format(RID p_texture) const = 0;

	typedef void (*TextureDetectCallback)(void *);

	virtual void texture_set_detect_3d_callback(RID p_texture, TextureDetectCallback p_callback, void *p_userdata) = 0;
	virtual void texture_set_detect_normal_callback(RID p_texture, TextureDetectCallback p_callback, void *p_userdata) = 0;

	enum TextureDetectRoughnessChannel {
		TEXTURE_DETECT_ROUGHNESS_R,
		TEXTURE_DETECT_ROUGHNESS_G,
		TEXTURE_DETECT_ROUGHNESS_B,
		TEXTURE_DETECT_ROUGHNESS_A,
		TEXTURE_DETECT_ROUGHNESS_GRAY,
	};

	typedef void (*TextureDetectRoughnessCallback)(void *, const String &, TextureDetectRoughnessChannel);
	virtual void texture_set_detect_roughness_callback(RID p_texture, TextureDetectRoughnessCallback p_callback, void *p_userdata) = 0;

	struct TextureInfo {
		RID texture;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		Image::Format format;
		int bytes;
		String path;
	};

	virtual void texture_debug_usage(List<TextureInfo> *r_info) = 0;
	Array _texture_debug_usage_bind();

	virtual void texture_set_force_redraw_if_visible(RID p_texture, bool p_enable) = 0;

	virtual uint64_t texture_get_native_handle(RID p_texture, bool p_srgb = false) const = 0;

	/* SHADER API */

	enum ShaderMode {
		SHADER_CANVAS_ITEM,
		SHADER_MAX
	};

	virtual RID shader_create() = 0;

	virtual void shader_set_code(RID p_shader, const String &p_code) = 0;
	virtual void shader_set_path_hint(RID p_shader, const String &p_path) = 0;
	virtual String shader_get_code(RID p_shader) const = 0;
	virtual void get_shader_parameter_list(RID p_shader, List<PropertyInfo> *p_param_list) const = 0;
	virtual Variant shader_get_parameter_default(RID p_shader, const StringName &p_param) const = 0;

	virtual void shader_set_default_texture_parameter(RID p_shader, const StringName &p_name, RID p_texture, int p_index = 0) = 0;
	virtual RID shader_get_default_texture_parameter(RID p_shader, const StringName &p_name, int p_index = 0) const = 0;

	struct ShaderNativeSourceCode {
		struct Version {
			struct Stage {
				String name;
				String code;
			};
			Vector<Stage> stages;
		};
		Vector<Version> versions;
	};

	virtual ShaderNativeSourceCode shader_get_native_source_code(RID p_shader) const = 0;

	/* COMMON MATERIAL API */

	virtual RID material_create() = 0;

	virtual void material_set_shader(RID p_shader_material, RID p_shader) = 0;

	virtual void material_set_param(RID p_material, const StringName &p_param, const Variant &p_value) = 0;
	virtual Variant material_get_param(RID p_material, const StringName &p_param) const = 0;

	/* MESH API */

	enum ArrayType {
		ARRAY_VERTEX = 0, // RG32F (2D), RGB32F, RGBA16 (compressed)
		ARRAY_NORMAL = 1, // RG16
		ARRAY_TANGENT = 2, // BA16 (with normal) or A16 (with vertex, when compressed)
		ARRAY_COLOR = 3, // RGBA8
		ARRAY_TEX_UV = 4, // RG32F or RG16
		ARRAY_TEX_UV2 = 5, // RG32F or RG16
		ARRAY_CUSTOM0 = 6, // Depends on ArrayCustomFormat.
		ARRAY_CUSTOM1 = 7,
		ARRAY_CUSTOM2 = 8,
		ARRAY_CUSTOM3 = 9,
		ARRAY_INDEX = 12, // 16 or 32 bits depending on length > 0xFFFF.
		ARRAY_MAX = 13
	};

	enum {
		ARRAY_CUSTOM_COUNT = ARRAY_INDEX - ARRAY_CUSTOM0
	};

	enum ArrayCustomFormat {
		ARRAY_CUSTOM_RGBA8_UNORM,
		ARRAY_CUSTOM_RGBA8_SNORM,
		ARRAY_CUSTOM_RG_HALF,
		ARRAY_CUSTOM_RGBA_HALF,
		ARRAY_CUSTOM_R_FLOAT,
		ARRAY_CUSTOM_RG_FLOAT,
		ARRAY_CUSTOM_RGB_FLOAT,
		ARRAY_CUSTOM_RGBA_FLOAT,
		ARRAY_CUSTOM_MAX
	};

	enum ArrayFormat : uint64_t {
		/* ARRAY FORMAT FLAGS */
		ARRAY_FORMAT_VERTEX = 1 << ARRAY_VERTEX,
		ARRAY_FORMAT_NORMAL = 1 << ARRAY_NORMAL,
		ARRAY_FORMAT_TANGENT = 1 << ARRAY_TANGENT,
		ARRAY_FORMAT_COLOR = 1 << ARRAY_COLOR,
		ARRAY_FORMAT_TEX_UV = 1 << ARRAY_TEX_UV,
		ARRAY_FORMAT_TEX_UV2 = 1 << ARRAY_TEX_UV2,
		ARRAY_FORMAT_CUSTOM0 = 1 << ARRAY_CUSTOM0,
		ARRAY_FORMAT_CUSTOM1 = 1 << ARRAY_CUSTOM1,
		ARRAY_FORMAT_CUSTOM2 = 1 << ARRAY_CUSTOM2,
		ARRAY_FORMAT_CUSTOM3 = 1 << ARRAY_CUSTOM3,
		ARRAY_FORMAT_INDEX = 1 << ARRAY_INDEX,

		ARRAY_FORMAT_BLEND_SHAPE_MASK = ARRAY_FORMAT_VERTEX | ARRAY_FORMAT_NORMAL | ARRAY_FORMAT_TANGENT,

		ARRAY_FORMAT_CUSTOM_BASE = (ARRAY_INDEX + 1),
		ARRAY_FORMAT_CUSTOM_BITS = 3,
		ARRAY_FORMAT_CUSTOM_MASK = 0x7,
		ARRAY_FORMAT_CUSTOM0_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + 0),
		ARRAY_FORMAT_CUSTOM1_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS),
		ARRAY_FORMAT_CUSTOM2_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS * 2),
		ARRAY_FORMAT_CUSTOM3_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS * 3),

		ARRAY_COMPRESS_FLAGS_BASE = (ARRAY_INDEX + 1 + 12),

		ARRAY_FLAG_USE_2D_VERTICES = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 0),
		ARRAY_FLAG_USE_DYNAMIC_UPDATE = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 1),

		ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 3),

		ARRAY_FLAG_COMPRESS_ATTRIBUTES = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 4),
		// We leave enough room for up to 5 more compression flags.

		ARRAY_FLAG_FORMAT_VERSION_BASE = ARRAY_COMPRESS_FLAGS_BASE + 10,
		ARRAY_FLAG_FORMAT_VERSION_SHIFT = ARRAY_FLAG_FORMAT_VERSION_BASE,
		// When changes are made to the mesh format, add a new version and use it for the CURRENT_VERSION.
		ARRAY_FLAG_FORMAT_VERSION_1 = 0,
		ARRAY_FLAG_FORMAT_VERSION_2 = 1ULL << ARRAY_FLAG_FORMAT_VERSION_SHIFT,
		ARRAY_FLAG_FORMAT_CURRENT_VERSION = ARRAY_FLAG_FORMAT_VERSION_2,
		ARRAY_FLAG_FORMAT_VERSION_MASK = 0xFF, // 8 bits version
	};

	static_assert(sizeof(ArrayFormat) == 8, "ArrayFormat should be 64 bits long.");

	enum PrimitiveType {
		PRIMITIVE_POINTS,
		PRIMITIVE_LINES,
		PRIMITIVE_LINE_STRIP,
		PRIMITIVE_TRIANGLES,
		PRIMITIVE_TRIANGLE_STRIP,
		PRIMITIVE_MAX,
	};

	struct SurfaceData {
		PrimitiveType primitive = PRIMITIVE_MAX;

		uint64_t format = ARRAY_FLAG_FORMAT_CURRENT_VERSION;
		Vector<uint8_t> vertex_data; // Vertex, Normal, Tangent (change with skinning, blendshape).
		Vector<uint8_t> attribute_data; // Color, UV, UV2, Custom0-3.
		Vector<uint8_t> skin_data; // Bone index, Bone weight.
		uint32_t vertex_count = 0;
		Vector<uint8_t> index_data;
		uint32_t index_count = 0;

		AABB aabb;
		struct LOD {
			float edge_length = 0.0f;
			Vector<uint8_t> index_data;
		};
		Vector<LOD> lods;

		Vector<uint8_t> blend_shape_data;

		Vector4 uv_scale;

		RID material;
	};

	enum BlendShapeMode {
		BLEND_SHAPE_MODE_NORMALIZED,
		BLEND_SHAPE_MODE_RELATIVE,
	};

	/* VIEWPORT API */

	enum CanvasItemTextureFilter {
		CANVAS_ITEM_TEXTURE_FILTER_DEFAULT, // Uses canvas item setting for draw command, uses global setting for canvas item.
		CANVAS_ITEM_TEXTURE_FILTER_NEAREST,
		CANVAS_ITEM_TEXTURE_FILTER_LINEAR,
		CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS,
		CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS,
		CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC,
		CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC,
		CANVAS_ITEM_TEXTURE_FILTER_MAX
	};

	enum CanvasItemTextureRepeat {
		CANVAS_ITEM_TEXTURE_REPEAT_DEFAULT, // Uses canvas item setting for draw command, uses global setting for canvas item.
		CANVAS_ITEM_TEXTURE_REPEAT_DISABLED,
		CANVAS_ITEM_TEXTURE_REPEAT_ENABLED,
		CANVAS_ITEM_TEXTURE_REPEAT_MIRROR,
		CANVAS_ITEM_TEXTURE_REPEAT_MAX,
	};

	virtual RID viewport_create() = 0;

	virtual void viewport_set_size(RID p_viewport, int p_width, int p_height) = 0;
	virtual void viewport_set_active(RID p_viewport, bool p_active) = 0;
	virtual void viewport_set_parent_viewport(RID p_viewport, RID p_parent_viewport) = 0;
	virtual void viewport_set_canvas_cull_mask(RID p_viewport, uint32_t p_canvas_cull_mask) = 0;

	virtual void viewport_attach_to_screen(RID p_viewport, const Rect2 &p_rect = Rect2(), DisplayServer::WindowID p_screen = DisplayServer::MAIN_WINDOW_ID) = 0;
	virtual void viewport_set_render_direct_to_screen(RID p_viewport, bool p_enable) = 0;

	enum ViewportUpdateMode {
		VIEWPORT_UPDATE_DISABLED,
		VIEWPORT_UPDATE_ONCE, // Then goes to disabled, must be manually updated.
		VIEWPORT_UPDATE_WHEN_VISIBLE, // Default
		VIEWPORT_UPDATE_WHEN_PARENT_VISIBLE,
		VIEWPORT_UPDATE_ALWAYS
	};

	virtual void viewport_set_update_mode(RID p_viewport, ViewportUpdateMode p_mode) = 0;

	enum ViewportClearMode {
		VIEWPORT_CLEAR_ALWAYS,
		VIEWPORT_CLEAR_NEVER,
		VIEWPORT_CLEAR_ONLY_NEXT_FRAME
	};

	virtual void viewport_set_clear_mode(RID p_viewport, ViewportClearMode p_clear_mode) = 0;

	virtual RID viewport_get_render_target(RID p_viewport) const = 0;
	virtual RID viewport_get_texture(RID p_viewport) const = 0;

	virtual void viewport_set_disable_2d(RID p_viewport, bool p_disable) = 0;

	virtual void viewport_attach_camera(RID p_viewport, RID p_camera) = 0;
	virtual void viewport_attach_canvas(RID p_viewport, RID p_canvas) = 0;
	virtual void viewport_remove_canvas(RID p_viewport, RID p_canvas) = 0;
	virtual void viewport_set_canvas_transform(RID p_viewport, RID p_canvas, const Transform2D &p_offset) = 0;
	virtual void viewport_set_transparent_background(RID p_viewport, bool p_enabled) = 0;
	virtual void viewport_set_snap_2d_transforms_to_pixel(RID p_viewport, bool p_enabled) = 0;
	virtual void viewport_set_snap_2d_vertices_to_pixel(RID p_viewport, bool p_enabled) = 0;

	virtual void viewport_set_default_canvas_item_texture_filter(RID p_viewport, CanvasItemTextureFilter p_filter) = 0;
	virtual void viewport_set_default_canvas_item_texture_repeat(RID p_viewport, CanvasItemTextureRepeat p_repeat) = 0;

	virtual void viewport_set_global_canvas_transform(RID p_viewport, const Transform2D &p_transform) = 0;
	virtual void viewport_set_canvas_stacking(RID p_viewport, RID p_canvas, int p_layer, int p_sublayer) = 0;

	enum ViewportSDFOversize {
		VIEWPORT_SDF_OVERSIZE_100_PERCENT,
		VIEWPORT_SDF_OVERSIZE_120_PERCENT,
		VIEWPORT_SDF_OVERSIZE_150_PERCENT,
		VIEWPORT_SDF_OVERSIZE_200_PERCENT,
		VIEWPORT_SDF_OVERSIZE_MAX
	};

	enum ViewportSDFScale {
		VIEWPORT_SDF_SCALE_100_PERCENT,
		VIEWPORT_SDF_SCALE_50_PERCENT,
		VIEWPORT_SDF_SCALE_25_PERCENT,
		VIEWPORT_SDF_SCALE_MAX
	};

	virtual void viewport_set_sdf_oversize_and_scale(RID p_viewport, ViewportSDFOversize p_oversize, ViewportSDFScale p_scale) = 0;

	enum ViewportMSAA {
		VIEWPORT_MSAA_DISABLED,
		VIEWPORT_MSAA_2X,
		VIEWPORT_MSAA_4X,
		VIEWPORT_MSAA_8X,
		VIEWPORT_MSAA_MAX,
	};

	virtual void viewport_set_msaa_2d(RID p_viewport, ViewportMSAA p_msaa) = 0;

	enum ViewportRenderInfo {
		VIEWPORT_RENDER_INFO_OBJECTS_IN_FRAME,
		VIEWPORT_RENDER_INFO_PRIMITIVES_IN_FRAME,
		VIEWPORT_RENDER_INFO_DRAW_CALLS_IN_FRAME,
		VIEWPORT_RENDER_INFO_MAX,
	};

	enum ViewportRenderInfoType {
		VIEWPORT_RENDER_INFO_TYPE_VISIBLE,
		VIEWPORT_RENDER_INFO_TYPE_SHADOW,
		VIEWPORT_RENDER_INFO_TYPE_MAX
	};

	virtual int viewport_get_render_info(RID p_viewport, ViewportRenderInfoType p_type, ViewportRenderInfo p_info) = 0;

	virtual void viewport_set_measure_render_time(RID p_viewport, bool p_enable) = 0;
	virtual double viewport_get_measured_render_time_cpu(RID p_viewport) const = 0;
	virtual double viewport_get_measured_render_time_gpu(RID p_viewport) const = 0;

	virtual RID viewport_find_from_screen_attachment(DisplayServer::WindowID p_id = DisplayServer::MAIN_WINDOW_ID) const = 0;

	/* INSTANCING API */

	enum InstanceType {
		INSTANCE_NONE,
		INSTANCE_MAX,
	};

	// virtual RID instance_create() = 0;

	enum VisibilityRangeFadeMode {
		VISIBILITY_RANGE_FADE_DISABLED,
		VISIBILITY_RANGE_FADE_SELF,
		VISIBILITY_RANGE_FADE_DEPENDENCIES,
	};

	/* CANVAS (2D) */

	virtual RID canvas_create() = 0;
	virtual void canvas_set_item_mirroring(RID p_canvas, RID p_item, const Point2 &p_mirroring) = 0;
	virtual void canvas_set_modulate(RID p_canvas, const Color &p_color) = 0;
	virtual void canvas_set_parent(RID p_canvas, RID p_parent, float p_scale) = 0;

	virtual void canvas_set_disable_scale(bool p_disable) = 0;

	/* CANVAS TEXTURE */
	virtual RID canvas_texture_create() = 0;

	enum CanvasTextureChannel {
		CANVAS_TEXTURE_CHANNEL_DIFFUSE,
		CANVAS_TEXTURE_CHANNEL_NORMAL,
		CANVAS_TEXTURE_CHANNEL_SPECULAR,
	};
	virtual void canvas_texture_set_channel(RID p_canvas_texture, CanvasTextureChannel p_channel, RID p_texture) = 0;
	virtual void canvas_texture_set_shading_parameters(RID p_canvas_texture, const Color &p_base_color, float p_shininess) = 0;

	// Takes effect only for new draw commands.
	virtual void canvas_texture_set_texture_filter(RID p_canvas_texture, CanvasItemTextureFilter p_filter) = 0;
	virtual void canvas_texture_set_texture_repeat(RID p_canvas_texture, CanvasItemTextureRepeat p_repeat) = 0;

	/* CANVAS ITEM */

	virtual RID canvas_item_create() = 0;
	virtual void canvas_item_set_parent(RID p_item, RID p_parent) = 0;

	virtual void canvas_item_set_default_texture_filter(RID p_item, CanvasItemTextureFilter p_filter) = 0;
	virtual void canvas_item_set_default_texture_repeat(RID p_item, CanvasItemTextureRepeat p_repeat) = 0;

	virtual void canvas_item_set_visible(RID p_item, bool p_visible) = 0;
	virtual void canvas_item_set_light_mask(RID p_item, int p_mask) = 0;

	virtual void canvas_item_set_update_when_visible(RID p_item, bool p_update) = 0;

	virtual void canvas_item_set_transform(RID p_item, const Transform2D &p_transform) = 0;
	virtual void canvas_item_set_clip(RID p_item, bool p_clip) = 0;
	virtual void canvas_item_set_distance_field_mode(RID p_item, bool p_enable) = 0;
	virtual void canvas_item_set_custom_rect(RID p_item, bool p_custom_rect, const Rect2 &p_rect = Rect2()) = 0;
	virtual void canvas_item_set_modulate(RID p_item, const Color &p_color) = 0;
	virtual void canvas_item_set_self_modulate(RID p_item, const Color &p_color) = 0;
	virtual void canvas_item_set_visibility_layer(RID p_item, uint32_t p_visibility_layer) = 0;

	virtual void canvas_item_set_draw_behind_parent(RID p_item, bool p_enable) = 0;

	enum NinePatchAxisMode {
		NINE_PATCH_STRETCH,
		NINE_PATCH_TILE,
		NINE_PATCH_TILE_FIT,
	};

	virtual void canvas_item_add_line(RID p_item, const Point2 &p_from, const Point2 &p_to, const Color &p_color, float p_width = -1.0, bool p_antialiased = false) = 0;
	virtual void canvas_item_add_polyline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width = -1.0, bool p_antialiased = false) = 0;
	virtual void canvas_item_add_multiline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width = -1.0) = 0;
	virtual void canvas_item_add_rect(RID p_item, const Rect2 &p_rect, const Color &p_color) = 0;
	virtual void canvas_item_add_circle(RID p_item, const Point2 &p_pos, float p_radius, const Color &p_color) = 0;
	virtual void canvas_item_add_texture_rect(RID p_item, const Rect2 &p_rect, RID p_texture, bool p_tile = false, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false) = 0;
	virtual void canvas_item_add_texture_rect_region(RID p_item, const Rect2 &p_rect, RID p_texture, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, bool p_clip_uv = false) = 0;
	virtual void canvas_item_add_msdf_texture_rect_region(RID p_item, const Rect2 &p_rect, RID p_texture, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1), int p_outline_size = 0, float p_px_range = 1.0, float p_scale = 1.0) = 0;
	virtual void canvas_item_add_lcd_texture_rect_region(RID p_item, const Rect2 &p_rect, RID p_texture, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1)) = 0;
	virtual void canvas_item_add_nine_patch(RID p_item, const Rect2 &p_rect, const Rect2 &p_source, RID p_texture, const Vector2 &p_topleft, const Vector2 &p_bottomright, NinePatchAxisMode p_x_axis_mode = NINE_PATCH_STRETCH, NinePatchAxisMode p_y_axis_mode = NINE_PATCH_STRETCH, bool p_draw_center = true, const Color &p_modulate = Color(1, 1, 1)) = 0;
	virtual void canvas_item_add_primitive(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs, RID p_texture) = 0;
	virtual void canvas_item_add_polygon(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), RID p_texture = RID()) = 0;
	virtual void canvas_item_add_triangle_array(RID p_item, const Vector<int> &p_indices, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), RID p_texture = RID(), int p_count = -1) = 0;
	virtual void canvas_item_add_set_transform(RID p_item, const Transform2D &p_transform) = 0;
	virtual void canvas_item_add_clip_ignore(RID p_item, bool p_ignore) = 0;
	virtual void canvas_item_add_animation_slice(RID p_item, double p_animation_length, double p_slice_begin, double p_slice_end, double p_offset) = 0;

	virtual void canvas_item_set_sort_children_by_y(RID p_item, bool p_enable) = 0;
	virtual void canvas_item_set_z_index(RID p_item, int p_z) = 0;
	virtual void canvas_item_set_z_as_relative_to_parent(RID p_item, bool p_enable) = 0;
	virtual void canvas_item_set_copy_to_backbuffer(RID p_item, bool p_enable, const Rect2 &p_rect) = 0;

	virtual void canvas_item_clear(RID p_item) = 0;
	virtual void canvas_item_set_draw_index(RID p_item, int p_index) = 0;

	virtual void canvas_item_set_material(RID p_item, RID p_material) = 0;

	virtual void canvas_item_set_use_parent_material(RID p_item, bool p_enable) = 0;

	virtual void canvas_item_set_visibility_notifier(RID p_item, bool p_enable, const Rect2 &p_area, const Callable &p_enter_callbable, const Callable &p_exit_callable) = 0;

	enum CanvasGroupMode {
		CANVAS_GROUP_MODE_DISABLED,
		CANVAS_GROUP_MODE_CLIP_ONLY,
		CANVAS_GROUP_MODE_CLIP_AND_DRAW,
		CANVAS_GROUP_MODE_TRANSPARENT,
	};

	virtual void canvas_item_set_canvas_group_mode(RID p_item, CanvasGroupMode p_mode, float p_clear_margin = 5.0, bool p_fit_empty = false, float p_fit_margin = 0.0, bool p_blur_mipmaps = false) = 0;

	virtual void canvas_item_set_debug_redraw(bool p_enabled) = 0;
	virtual bool canvas_item_get_debug_redraw() const = 0;

	/* CANVAS LIGHT */
	virtual RID canvas_light_create() = 0;

	enum CanvasLightMode {
		CANVAS_LIGHT_MODE_POINT,
		CANVAS_LIGHT_MODE_DIRECTIONAL,
	};

	virtual void canvas_light_set_mode(RID p_light, CanvasLightMode p_mode) = 0;

	virtual void canvas_light_attach_to_canvas(RID p_light, RID p_canvas) = 0;
	virtual void canvas_light_set_enabled(RID p_light, bool p_enabled) = 0;
	virtual void canvas_light_set_transform(RID p_light, const Transform2D &p_transform) = 0;
	virtual void canvas_light_set_color(RID p_light, const Color &p_color) = 0;
	virtual void canvas_light_set_height(RID p_light, float p_height) = 0;
	virtual void canvas_light_set_energy(RID p_light, float p_energy) = 0;
	virtual void canvas_light_set_z_range(RID p_light, int p_min_z, int p_max_z) = 0;
	virtual void canvas_light_set_layer_range(RID p_light, int p_min_layer, int p_max_layer) = 0;
	virtual void canvas_light_set_item_cull_mask(RID p_light, int p_mask) = 0;
	virtual void canvas_light_set_item_shadow_cull_mask(RID p_light, int p_mask) = 0;

	virtual void canvas_light_set_directional_distance(RID p_light, float p_distance) = 0;

	virtual void canvas_light_set_texture_scale(RID p_light, float p_scale) = 0;
	virtual void canvas_light_set_texture(RID p_light, RID p_texture) = 0;
	virtual void canvas_light_set_texture_offset(RID p_light, const Vector2 &p_offset) = 0;

	enum CanvasLightBlendMode {
		CANVAS_LIGHT_BLEND_MODE_ADD,
		CANVAS_LIGHT_BLEND_MODE_SUB,
		CANVAS_LIGHT_BLEND_MODE_MIX,
	};

	virtual void canvas_light_set_blend_mode(RID p_light, CanvasLightBlendMode p_mode) = 0;

	enum CanvasLightShadowFilter {
		CANVAS_LIGHT_FILTER_NONE,
		CANVAS_LIGHT_FILTER_PCF5,
		CANVAS_LIGHT_FILTER_PCF13,
		CANVAS_LIGHT_FILTER_MAX
	};

	virtual void canvas_light_set_shadow_enabled(RID p_light, bool p_enabled) = 0;
	virtual void canvas_light_set_shadow_filter(RID p_light, CanvasLightShadowFilter p_filter) = 0;
	virtual void canvas_light_set_shadow_color(RID p_light, const Color &p_color) = 0;
	virtual void canvas_light_set_shadow_smooth(RID p_light, float p_smooth) = 0;

	/* CANVAS LIGHT OCCLUDER */

	virtual RID canvas_light_occluder_create() = 0;
	virtual void canvas_light_occluder_attach_to_canvas(RID p_occluder, RID p_canvas) = 0;
	virtual void canvas_light_occluder_set_enabled(RID p_occluder, bool p_enabled) = 0;
	virtual void canvas_light_occluder_set_polygon(RID p_occluder, RID p_polygon) = 0;
	virtual void canvas_light_occluder_set_as_sdf_collision(RID p_occluder, bool p_enable) = 0;
	virtual void canvas_light_occluder_set_transform(RID p_occluder, const Transform2D &p_xform) = 0;
	virtual void canvas_light_occluder_set_light_mask(RID p_occluder, int p_mask) = 0;

	/* CANVAS LIGHT OCCLUDER POLYGON */

	virtual RID canvas_occluder_polygon_create() = 0;
	virtual void canvas_occluder_polygon_set_shape(RID p_occluder_polygon, const Vector<Vector2> &p_shape, bool p_closed) = 0;

	enum CanvasOccluderPolygonCullMode {
		CANVAS_OCCLUDER_POLYGON_CULL_DISABLED,
		CANVAS_OCCLUDER_POLYGON_CULL_CLOCKWISE,
		CANVAS_OCCLUDER_POLYGON_CULL_COUNTER_CLOCKWISE,
	};

	virtual void canvas_occluder_polygon_set_cull_mode(RID p_occluder_polygon, CanvasOccluderPolygonCullMode p_mode) = 0;

	virtual void canvas_set_shadow_texture_size(int p_size) = 0;

	Rect2 debug_canvas_item_get_rect(RID p_item);
	virtual Rect2 _debug_canvas_item_get_rect(RID p_item) = 0;

	/* GLOBAL SHADER UNIFORMS */

	enum GlobalShaderParameterType {
		GLOBAL_VAR_TYPE_BOOL,
		GLOBAL_VAR_TYPE_BVEC2,
		GLOBAL_VAR_TYPE_BVEC3,
		GLOBAL_VAR_TYPE_BVEC4,
		GLOBAL_VAR_TYPE_INT,
		GLOBAL_VAR_TYPE_IVEC2,
		GLOBAL_VAR_TYPE_IVEC3,
		GLOBAL_VAR_TYPE_IVEC4,
		GLOBAL_VAR_TYPE_RECT2I,
		GLOBAL_VAR_TYPE_UINT,
		GLOBAL_VAR_TYPE_UVEC2,
		GLOBAL_VAR_TYPE_UVEC3,
		GLOBAL_VAR_TYPE_UVEC4,
		GLOBAL_VAR_TYPE_FLOAT,
		GLOBAL_VAR_TYPE_VEC2,
		GLOBAL_VAR_TYPE_VEC3,
		GLOBAL_VAR_TYPE_VEC4,
		GLOBAL_VAR_TYPE_COLOR,
		GLOBAL_VAR_TYPE_RECT2,
		GLOBAL_VAR_TYPE_MAT2,
		GLOBAL_VAR_TYPE_MAT3,
		GLOBAL_VAR_TYPE_MAT4,
		GLOBAL_VAR_TYPE_TRANSFORM_2D,
		GLOBAL_VAR_TYPE_TRANSFORM,
		GLOBAL_VAR_TYPE_SAMPLER2D,
		GLOBAL_VAR_TYPE_MAX
	};

	virtual void global_shader_parameter_add(const StringName &p_name, GlobalShaderParameterType p_type, const Variant &p_value) = 0;
	virtual void global_shader_parameter_remove(const StringName &p_name) = 0;
	virtual Vector<StringName> global_shader_parameter_get_list() const = 0;

	virtual void global_shader_parameter_set(const StringName &p_name, const Variant &p_value) = 0;
	virtual void global_shader_parameter_set_override(const StringName &p_name, const Variant &p_value) = 0;

	virtual Variant global_shader_parameter_get(const StringName &p_name) const = 0;
	virtual GlobalShaderParameterType global_shader_parameter_get_type(const StringName &p_name) const = 0;

	virtual void global_shader_parameters_load_settings(bool p_load_textures) = 0;
	virtual void global_shader_parameters_clear() = 0;

	static int global_shader_uniform_type_get_shader_datatype(GlobalShaderParameterType p_type);

	/* FREE */

	virtual void free(RID p_rid) = 0; // Free RIDs associated with the rendering server.

	/* EVENT QUEUING */

	virtual void request_frame_drawn_callback(const Callable &p_callable) = 0;

	virtual void draw(bool p_swap_buffers = true, double frame_step = 0.0) = 0;
	virtual void sync() = 0;
	virtual bool has_changed() const = 0;
	virtual void init();
	virtual void finish() = 0;

	/* STATUS INFORMATION */

	enum RenderingInfo {
		RENDERING_INFO_TOTAL_OBJECTS_IN_FRAME,
		RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME,
		RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME,
		RENDERING_INFO_TEXTURE_MEM_USED,
		RENDERING_INFO_BUFFER_MEM_USED,
		RENDERING_INFO_VIDEO_MEM_USED,
		RENDERING_INFO_MAX
	};

	virtual uint64_t get_rendering_info(RenderingInfo p_info) = 0;
	virtual String get_video_adapter_name() const = 0;
	virtual String get_video_adapter_vendor() const = 0;
	virtual String get_video_adapter_api_version() const = 0;

	struct FrameProfileArea {
		String name;
		double gpu_msec;
		double cpu_msec;
	};

	virtual void set_frame_profiling_enabled(bool p_enable) = 0;
	virtual Vector<FrameProfileArea> get_frame_profile() = 0;
	virtual uint64_t get_frame_profile_frame() = 0;

	virtual double get_frame_setup_time_cpu() const = 0;

	/* TESTING */

	virtual RID get_test_texture();
	virtual RID get_white_texture();

	virtual void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true) = 0;
	virtual Color get_default_clear_color() = 0;
	virtual void set_default_clear_color(const Color &p_color) = 0;

	enum Features {
		FEATURE_SHADERS,
		FEATURE_MULTITHREADED,
	};

	virtual bool has_feature(Features p_feature) const = 0;

	virtual bool has_os_feature(const String &p_feature) const = 0;

	virtual void set_debug_generate_wireframes(bool p_generate) = 0;

	virtual void call_set_vsync_mode(DisplayServer::VSyncMode p_mode, DisplayServer::WindowID p_window) = 0;

	virtual bool is_low_end() const = 0;

	virtual void set_print_gpu_profile(bool p_enable) = 0;

	virtual Size2i get_maximum_viewport_size() const = 0;

	bool is_render_loop_enabled() const;
	void set_render_loop_enabled(bool p_enabled);

	virtual void call_on_render_thread(const Callable &p_callable) = 0;

	RenderingServer();
	virtual ~RenderingServer();

private:
	// Binder helpers
	TypedArray<Dictionary> _shader_get_shader_parameter_list(RID p_shader) const;
};

// Make variant understand the enums.
VARIANT_ENUM_CAST(RenderingServer::ShaderMode);
VARIANT_ENUM_CAST(RenderingServer::ArrayType);
VARIANT_BITFIELD_CAST(RenderingServer::ArrayFormat);
VARIANT_ENUM_CAST(RenderingServer::ArrayCustomFormat);
VARIANT_ENUM_CAST(RenderingServer::PrimitiveType);
VARIANT_ENUM_CAST(RenderingServer::BlendShapeMode);
VARIANT_ENUM_CAST(RenderingServer::ViewportUpdateMode);
VARIANT_ENUM_CAST(RenderingServer::ViewportClearMode);
VARIANT_ENUM_CAST(RenderingServer::ViewportMSAA);
VARIANT_ENUM_CAST(RenderingServer::ViewportRenderInfo);
VARIANT_ENUM_CAST(RenderingServer::ViewportRenderInfoType);
VARIANT_ENUM_CAST(RenderingServer::ViewportSDFOversize);
VARIANT_ENUM_CAST(RenderingServer::ViewportSDFScale);
VARIANT_ENUM_CAST(RenderingServer::InstanceType);
VARIANT_ENUM_CAST(RenderingServer::VisibilityRangeFadeMode);
VARIANT_ENUM_CAST(RenderingServer::NinePatchAxisMode);
VARIANT_ENUM_CAST(RenderingServer::CanvasItemTextureFilter);
VARIANT_ENUM_CAST(RenderingServer::CanvasItemTextureRepeat);
VARIANT_ENUM_CAST(RenderingServer::CanvasGroupMode);
VARIANT_ENUM_CAST(RenderingServer::CanvasLightMode);
VARIANT_ENUM_CAST(RenderingServer::CanvasLightBlendMode);
VARIANT_ENUM_CAST(RenderingServer::CanvasLightShadowFilter);
VARIANT_ENUM_CAST(RenderingServer::CanvasOccluderPolygonCullMode);
VARIANT_ENUM_CAST(RenderingServer::GlobalShaderParameterType);
VARIANT_ENUM_CAST(RenderingServer::RenderingInfo);
VARIANT_ENUM_CAST(RenderingServer::Features);
VARIANT_ENUM_CAST(RenderingServer::CanvasTextureChannel);

// Alias to make it easier to use.
#define RS RenderingServer

#endif // RENDERING_SERVER_H
