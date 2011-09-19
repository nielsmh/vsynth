#include "vsynth.h"

HEADER_START


struct StandardFrameVirtual {
	struct FrameVirtual base;
};

extern struct StandardFrameVirtual stdframe_vtable;

struct StandardFrame {
	struct Frame base;

	size_t stride[4];
	void *data[4];

	void *data_baseptr;
};
typedef struct StandardFrame *StandardFrame;


VSYNTH_API(StandardFrame) Stdframe_New(int pixfmt, size_t width, size_t height);
VSYNTH_API(StandardFrame) Stdframe_Get(Frame frame);


HEADER_END
