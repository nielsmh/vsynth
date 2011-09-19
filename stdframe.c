#define VSYNTH_API(rettype) __declspec(dllexport) rettype __stdcall

#include "stdframe.h"
#include <stdlib.h>
#include <assert.h>

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
	assert(sf != NULL);

	// todo
	return NULL;
}

struct StandardFrameVirtual stdframe_vtable = {
	{
		Stdframe_destroy,
		Stdframe_clone
	},
};


VSYNTH_API(StandardFrame) Stdframe_New(enum StdframePixelFormat pixfmt, size_t width, size_t height)
{
	StandardFrame frame = (StandardFrame)malloc(sizeof(struct StandardFrame));

	frame->pixfmt = pixfmt;
	frame->base.width = width;
	frame->base.height = height;

	switch (pixfmt)
	{
	case STDPIXFMT_MONO8:
		frame->data_baseptr = malloc(width*height);
		frame->data[0] = frame->data_baseptr;
		frame->stride[0] = width;
		break;
	case STDPIXFMT_MONO16:
		frame->data_baseptr = malloc(width*height*2);
		frame->data[0] = frame->data_baseptr;
		frame->stride[0] = width*2;
		break;
	case STDPIXFMT_XRGB8:
	case STDPIXFMT_ARGB8:
		frame->data_baseptr = malloc(width*height*4);
		frame->data[0] = frame->data_baseptr;
		frame->stride[0] = width*4;
		break;
	case STDPIXFMT_XRGB16:
	case STDPIXFMT_ARGB16:
		frame->data_baseptr = malloc(width*height*8);
		frame->data[0] = frame->data_baseptr;
		frame->stride[0] = width*8;
		break;
	default:
		// lol
		break;
	}

	return frame;
}

VSYNTH_API(StandardFrame) Stdframe_Get(Frame frame)
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
