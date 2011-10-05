#pragma once

#include <vsynth/vsynth.h>

/*

This header defines a series of standard frame formats useful for most common
video processing tasks. It is not meant to be completely exhaustive.

Filters should attempt to do their work using stdframe formats, and only
define a new frame format if a stdframe format is inadequate.

*/

#ifdef __cplusplus
extern "C" {
#endif


/// Pixel formats supported by the stdframe type
enum Vs_StdframePixelFormat {
	// one channel, uint8_t pixels, gamma corrected, stored in plane 0, other planes unused
	STDPIXFMT_MONO8,
	// one channel, uint16_t pixels, linear gamma, stored in plane 0, other planes unused
	STDPIXFMT_MONO16,
	// XRGB, packed uint8_t channels, gamma corrected, stored in plane 0, other planes unused
	// pixels are 32 bit wide, stored as: <unused><red><green><blue>
	STDPIXFMT_XRGB8,
	// ARGB, packed uint8_t channels, gamma corrected, stored in plane 0, other planes unused
	// pixels are 32 bit wide, stored as: <alpha><red><green><blue>
	STDPIXFMT_ARGB8,
	// XRGB, packed uint16_t channels, linear gamma, stored in plane 0, other planes unused
	// pixels are 64 bit wide, stored as: <unused><red><green><blue>
	STDPIXFMT_XRGB16,
	// ARGB, packed uint16_t channels, linear gamma, stored in plane 0, other planes unused
	// pixels are 64 bit wide, stored as: <alpha><red><green><blue>
	STDPIXFMT_ARGB16,
	// YCrCb 4:4:4, uint8_t pixels, gamma corrected
	// plane 0 is Y, full resolution
	// plane 1 is Cr, full resolution
	// plane 2 is Cb, full resolution
	// plane 3 is unused
	STDPIXFMT_YCrCb8_444,
	// same as YCrCb8_444, but plane 3 is alpha at full resolution
	STDPIXFMT_YCrCbA8_444,
	// same as YCrCb8_444, but with uint16_t pixels (still gamma corrected)
	STDPIXFMT_YCrCb16_444,
	// same as YCrCb16_444, but plane 3 is alpha at full resolution
	STDPIXFMT_YCrCbA16_444,
	// YCrCb 4:2:2, uint8_t pixels, gamma corrected
	// plane 0 is Y, full resolution
	// plane 1 is Cr, half resolution in width, full in height
	// plane 2 is Cb, half resolution in width, full in height
	// plane 3 is unused
	STDPIXFMT_YCrCb8_422,
	// same as YCrCb8_422, but plane 3 is alpha at full resolution
	STDPIXFMT_YCrCbA8_422,
	// same as YCrCb8_422, but with uint16_t pixels (still gamma corrected)
	STDPIXFMT_YCrCb16_422,
	// same as YCrCb16_422, but plane 3 is alpha at full resolution
	STDPIXFMT_YCrCbA16_422,
	// YCrCb 4:2:0, uint8_t pixels, gamma corrected
	// plane 0 is Y, full resolution
	// plane 1 is Cr, half resolution in width, half resolution in height
	// plane 2 is Cb, half resolution in width, half resolution in height
	// plane 3 is unused
	STDPIXFMT_YCrCb8_420,
	// same as YCrCb8_420, but plane 3 is alpha at full resolution
	STDPIXFMT_YCrCbA8_420,
	// same as YCrCb8_420, but with uint16_t pixels (still gamma corrected)
	STDPIXFMT_YCrCb16_420,
	// same as YCrCb16_420, but plane 3 is alpha at full resolution
	STDPIXFMT_YCrCbA16_420,
	// sentinel marker counting the number of stdpixfmts
	STDPIXFMT_MAX
};


/// Type of stdframe objects
typedef struct Vs_StandardFrame *Vs_StandardFrame;

/// Vtable for stdframe objects
struct Vs_StandardFrameVirtual {
	struct Vs_FrameVirtual base;

	/// Crop the frame without reallocating or blitting
	VSYNTH_DECLARE_METHOD(void, crop)(Vs_StandardFrame frame, size_t left, size_t top, size_t width, size_t height);
};

/// Standard frame type useful for most common video processing
///
/// The StandardFrame type describes common mono, RGB and YCrCb formats.
struct Vs_StandardFrame {
	struct Vs_Frame base;

	/// Width of frame in pixels
	size_t width;
	/// Height of frame in pixels
	size_t height;

	/// Distance between start of scanlines for each plane
	ptrdiff_t stride[4];
	/// Pointers to each plane
	void *data[4];

	/// Pixel format of the frame
	enum Vs_StdframePixelFormat pixfmt;

	/// Internal: Pointer to raw memory allocation for the frame
	void *data_baseptr;
	/// Internal: Number of bytes allocated for the frame
	size_t data_rawsize;
};

/// The global vtable for StandardFrame objects
///
/// Every Frame that is a StandardFrame has its methods member pointing to
/// this vtable.
VSYNTH_EXTERN(struct Vs_StandardFrameVirtual) Vs_stdframe_vtable;

/// Description of a supported stdframe format for use in filter activation
struct Vs_StandardFrameTypeDescription {
	/// Base frame type description, must be initialised with a pointer to stdframe_vtable
	struct Vs_FrameTypeDescription base;
	/// Flag whether mid-stream resolution changes may occur
	///
	/// If this is set to non-zero on input, the activated filter should set
	/// it to zero if it can promise to never change resolution mid-stream.
	int allow_resolution_change;
	/// Flag whether mid-stream pixfmt changes may occur
	///
	/// If this is set to non-zero on input, the activated filter should set
	/// it to zero if it can promise to never change pixfmt mid-stream.
	int allow_pixfmt_change;
	/// Minimum width of picture required
	///
	/// On output, should be changed to indicate any promises stronger than
	/// the requirement.
	size_t minwidth;
	/// Maximum width of picture allowed
	///
	/// On output, should be changed to indicate any promises stronger than
	/// the requirement.
	size_t maxwidth;
	/// Minimum height of picture required
	///
	/// On output, should be changed to indicate any promises stronger than
	/// the requirement.
	size_t minheight;
	/// Maximum height of picture allowed
	///
	/// On output, should be changed to indicate any promises stronger than
	/// the requirement.
	size_t maxheight;
	/// Modulo requirement for picture width
	///
	/// 0 means no constraints on width. Non-zero means that frames must
	/// always have a width evenly divisible by this number.
	unsigned short width_modulo;
	/// Modulo requirement for picture height
	///
	/// 0 means no constraints on width. Non-zero means that frames must
	/// always have a height evenly divisible by this number.
	unsigned short height_modulo;
	/// Pointer to array of supported pixfmts
	///
	/// May be NULL if all pixfmts are supported.
	///
	/// Array must be terminated by a STDPIXFMT_MAX value.
	enum Vs_StdframePixelFormat *pixfmts;
};


/// Allocate a new stdframe with given properties
VSYNTH_API(Vs_StandardFrame) Vs_Stdframe_New(enum Vs_StdframePixelFormat pixfmt, size_t width, size_t height);
/// Check if a Frame is a stdframe, and return a StandardFrame pointer if it is
VSYNTH_API(Vs_StandardFrame) Vs_Stdframe_Get(Vs_Frame frame);


#ifdef __cplusplus
}
#endif
