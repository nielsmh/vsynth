#define VSYNTH_API(rettype) __declspec(dllexport) rettype __stdcall

#include <vsynth/stdframe.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static __inline size_t STDPIXFMT_pixelsize(enum StdframePixelFormat pixfmt)
{
	switch (pixfmt)
	{
	case STDPIXFMT_MONO8:
	case STDPIXFMT_YCrCb8_444:
	case STDPIXFMT_YCrCbA8_444:
	case STDPIXFMT_YCrCb8_422:
	case STDPIXFMT_YCrCbA8_422:
	case STDPIXFMT_YCrCb8_420:
	case STDPIXFMT_YCrCbA8_420:
		return 1;
	case STDPIXFMT_MONO16:
	case STDPIXFMT_YCrCb16_444:
	case STDPIXFMT_YCrCbA16_444:
	case STDPIXFMT_YCrCb16_422:
	case STDPIXFMT_YCrCbA16_422:
	case STDPIXFMT_YCrCb16_420:
	case STDPIXFMT_YCrCbA16_420:
		return 2;
	case STDPIXFMT_XRGB8:
	case STDPIXFMT_ARGB8:
		return 4;
	case STDPIXFMT_XRGB16:
	case STDPIXFMT_ARGB16:
		return 8;
	default:
		return 0;
	}
}

static __inline size_t STDPIXFMT_planecount(enum StdframePixelFormat pixfmt)
{
	switch (pixfmt)
	{
	case STDPIXFMT_MONO8:
	case STDPIXFMT_MONO16:
	case STDPIXFMT_XRGB8:
	case STDPIXFMT_ARGB8:
	case STDPIXFMT_XRGB16:
	case STDPIXFMT_ARGB16:
		return 1;
	case STDPIXFMT_YCrCb8_444:
	case STDPIXFMT_YCrCb8_422:
	case STDPIXFMT_YCrCb8_420:
	case STDPIXFMT_YCrCb16_444:
	case STDPIXFMT_YCrCb16_422:
	case STDPIXFMT_YCrCb16_420:
		return 3;
	case STDPIXFMT_YCrCbA8_444:
	case STDPIXFMT_YCrCbA8_422:
	case STDPIXFMT_YCrCbA8_420:
	case STDPIXFMT_YCrCbA16_444:
	case STDPIXFMT_YCrCbA16_422:
	case STDPIXFMT_YCrCbA16_420:
		return 4;
	default:
		return 0;
	}
}

static __inline size_t STDPIXFMT_planewidthscale(enum StdframePixelFormat pixfmt, size_t plane)
{
	switch (pixfmt)
	{
	case STDPIXFMT_MONO8:
	case STDPIXFMT_MONO16:
	case STDPIXFMT_XRGB8:
	case STDPIXFMT_ARGB8:
	case STDPIXFMT_XRGB16:
	case STDPIXFMT_ARGB16:
	case STDPIXFMT_YCrCb8_444:
	case STDPIXFMT_YCrCb16_444:
	case STDPIXFMT_YCrCbA8_444:
	case STDPIXFMT_YCrCbA16_444:
		return 1;
	case STDPIXFMT_YCrCb8_422:
	case STDPIXFMT_YCrCb8_420:
	case STDPIXFMT_YCrCb16_422:
	case STDPIXFMT_YCrCb16_420:
	case STDPIXFMT_YCrCbA8_422:
	case STDPIXFMT_YCrCbA8_420:
	case STDPIXFMT_YCrCbA16_422:
	case STDPIXFMT_YCrCbA16_420:
		if (plane == 0 || plane == 3)
			return 1;
		else
			return 2;
	default:
		return 0;
	}
}

static __inline size_t STDPIXFMT_planeheightscale(enum StdframePixelFormat pixfmt, size_t plane)
{
	switch (pixfmt)
	{
	case STDPIXFMT_MONO8:
	case STDPIXFMT_MONO16:
	case STDPIXFMT_XRGB8:
	case STDPIXFMT_ARGB8:
	case STDPIXFMT_XRGB16:
	case STDPIXFMT_ARGB16:
	case STDPIXFMT_YCrCb8_444:
	case STDPIXFMT_YCrCb16_444:
	case STDPIXFMT_YCrCbA8_444:
	case STDPIXFMT_YCrCbA16_444:
	case STDPIXFMT_YCrCb8_422:
	case STDPIXFMT_YCrCb16_422:
	case STDPIXFMT_YCrCbA8_422:
	case STDPIXFMT_YCrCbA16_422:
		return 1;
	case STDPIXFMT_YCrCb8_420:
	case STDPIXFMT_YCrCb16_420:
	case STDPIXFMT_YCrCbA8_420:
	case STDPIXFMT_YCrCbA16_420:
		if (plane == 0 || plane == 3)
			return 1;
		else
			return 2;
	default:
		return 0;
	}
}


static void VSYNTH_METHOD Stdframe_destroy(Frame frame)
{
	StandardFrame sf = Stdframe_Get(frame);
	assert(sf != NULL);

	free(sf->data_baseptr);
	free(sf);
}

static Frame VSYNTH_METHOD Stdframe_clone(Frame frame)
{
	StandardFrame sf = Stdframe_Get(frame);
	StandardFrame result = NULL;

	assert(sf != NULL);

	result = Stdframe_New(sf->pixfmt, sf->width, sf->height);
	if (result == NULL)
		return NULL;
	// fixme? check whether new frame has sama datasize as old?

	memcpy(result->data_baseptr, sf->data_baseptr, sf->data_rawsize);
	
	return (Frame)result;
}

static void VSYNTH_METHOD Stdframe_crop(StandardFrame frame, size_t left, size_t top, size_t width, size_t height)
{
	size_t pixelsize = STDPIXFMT_pixelsize(frame->pixfmt);
	size_t planes = STDPIXFMT_planecount(frame->pixfmt);
	size_t i;
	assert(pixelsize > 0);
	assert(planes > 0);

	if (left + width > frame->width)
		return;
	if (top + height > frame->height)
		return;

	frame->width = width;
	frame->height = height;

	for (i = 0; i < planes; i++)
	{
		frame->data[i] = (void*)( (char*)frame->data[i] +
			left * pixelsize / STDPIXFMT_planewidthscale(frame->pixfmt, i) +
			top * frame->stride[i] / STDPIXFMT_planeheightscale(frame->pixfmt, i)
		);
	}
}

struct StandardFrameVirtual stdframe_vtable = {
	{
		Stdframe_destroy,
		Stdframe_clone
	},
	Stdframe_crop,
};


VSYNTH_API(StandardFrame) Stdframe_New(enum StdframePixelFormat pixfmt, size_t width, size_t height)
{
	StandardFrame frame;

	size_t memreq = 0;
	ptrdiff_t stride[4] = {0};
	int planeheight[4] = {0};
	int pixelsize = 0;

	int i;

	switch (pixfmt)
	{
		// mono formats

	case STDPIXFMT_MONO8:
		pixelsize = 1;
		stride[0] = width;
		planeheight[0] = height;
		break;

	case STDPIXFMT_MONO16:
		pixelsize = 2;
		stride[0] = width*pixelsize;
		planeheight[0] = height;
		break;

		// rgb formats

	case STDPIXFMT_XRGB8:
	case STDPIXFMT_ARGB8:
		pixelsize = 4;
		stride[0] = width*pixelsize;
		planeheight[0] = height;
		break;

	case STDPIXFMT_XRGB16:
	case STDPIXFMT_ARGB16:
		pixelsize = 8;
		stride[0] = width*pixelsize;
		planeheight[0] = height;
		break;

		// 4:4:4 formats

	case STDPIXFMT_YCrCb8_444:
	case STDPIXFMT_YCrCbA8_444:
	case STDPIXFMT_YCrCb16_444:
	case STDPIXFMT_YCrCbA16_444:
		pixelsize = 1;
		if (pixfmt == STDPIXFMT_YCrCb16_444 || pixfmt == STDPIXFMT_YCrCbA16_444)
			pixelsize = 2;
		stride[0] = stride[1] = stride[2] = stride[3] = width * pixelsize;
		planeheight[0] = planeheight[1] = planeheight[2] = height;
		if (pixfmt == STDPIXFMT_YCrCbA8_444 || pixfmt == STDPIXFMT_YCrCbA16_444)
			planeheight[3] = height;
		break;

		// 4:2:2 formats

	case STDPIXFMT_YCrCb8_422:
	case STDPIXFMT_YCrCbA8_422:
	case STDPIXFMT_YCrCb16_422:
	case STDPIXFMT_YCrCbA16_422:
		pixelsize = 1;
		if (pixfmt == STDPIXFMT_YCrCb16_422 || pixfmt == STDPIXFMT_YCrCbA16_422)
			pixelsize = 2;
		stride[0] = stride[3] = width * pixelsize;
		stride[1] = stride[2] = (width+1)/2 * pixelsize;
		planeheight[0] = planeheight[1] = planeheight[2] = height;
		if (pixfmt == STDPIXFMT_YCrCbA8_422 || pixfmt == STDPIXFMT_YCrCbA16_422)
			planeheight[3] = height;
		break;

		// 4:2:0 formats

	case STDPIXFMT_YCrCb8_420:
	case STDPIXFMT_YCrCbA8_420:
	case STDPIXFMT_YCrCb16_420:
	case STDPIXFMT_YCrCbA16_420:
		pixelsize = 1;
		if (pixfmt == STDPIXFMT_YCrCb16_420 || pixfmt == STDPIXFMT_YCrCbA16_420)
			pixelsize = 2;
		stride[0] = stride[3] = width * pixelsize;
		stride[1] = stride[2] = (width+1)/2 * pixelsize;
		planeheight[0] = height;
		planeheight[1] = planeheight[2] = (height+1)/2;
		if (pixfmt == STDPIXFMT_YCrCbA8_420 || pixfmt == STDPIXFMT_YCrCbA16_420)
			planeheight[3] = height;
		break;

	default:
		// whoops, invalid!
		return NULL;
	}

	frame = (StandardFrame)malloc(sizeof(struct StandardFrame));
	frame->pixfmt = pixfmt;
	frame->width = width;
	frame->height = height;

	// calculate memory needed
	for (i = 0; i < 4; i++)
		memreq += stride[i] * planeheight[i];
	// allocate
	frame->data_baseptr = malloc(memreq);
	frame->data_rawsize = memreq;
	// store strides
	for (i = 0; i < 4; i++)
		frame->stride[i] = stride[i];
	// calculate and store plane locations
	frame->data[0] = frame->data_baseptr;
	for (i = 1; i < 4; i++)
		frame->data[i] = (void*)(((char*)frame->data[i-1]) + stride[i-1]*planeheight[i-1]);

	return frame;
}

__inline VSYNTH_API(StandardFrame) Stdframe_Get(Frame frame)
{
	if (frame->methods == &stdframe_vtable.base)
	{
		return (StandardFrame)frame;
	}
	else
	{
		return NULL;
	}
}
