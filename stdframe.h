#include "vsynth.h"

HEADER_START


enum StdframePixelFormat {
	// one channel, uint8_t pixels, gamma corrected, stored in plane 0, other planes unused
	STDPIXFMT_MONO8,
	// one channel, uint16_t pixels, linear gamma, stored in plane 0, other planes unused
	STDPIXFMT_MONO16,
	// XRGB, packed uint8_t channels, gamma corrected, stored in plane 0, other planes unused
	STDPIXFMT_XRGB8,
	// ARGB, packed uint8_t channels, gamma corrected, stored in plane 0, other planes unused
	STDPIXFMT_ARGB8,
	// XRGB, packed uint16_t channels, linear gamma, stored in plane 0, other planes unused
	STDPIXFMT_XRGB16,
	// ARGB, packed uint16_t channels, linear gamma, stored in plane 0, other planes unused
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
};


typedef struct StandardFrame *StandardFrame;

struct StandardFrameVirtual {
	struct FrameVirtual base;

	void (VSYNTH_METHOD *crop)(StandardFrame frame, size_t left, size_t top, size_t width, size_t height);
};

struct StandardFrame {
	struct Frame base;

	size_t stride[4];
	void *data[4];

	enum StdframePixelFormat pixfmt;

	void *data_baseptr;
	size_t data_rawsize;
};


VSYNTH_API(StandardFrame) Stdframe_New(enum StdframePixelFormat pixfmt, size_t width, size_t height);
VSYNTH_API(StandardFrame) Stdframe_Get(Frame frame);


HEADER_END
