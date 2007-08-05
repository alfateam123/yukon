
#include <yukon.h>

static unsigned int w, h;
static unsigned int targetFps;
static uint64_t timeStep;
static uint64_t timeNext;

static struct yukonPacket pkt;
static char buffer[2500 * 2500* 4];

void y4mWriteHeader(int fd, unsigned int width, unsigned int height, unsigned int fps)
{
	char header[4096];
	int n = snprintf(header, 4096, "YUV4MPEG2 W%d H%d F%d:1 Ip\n", width, height, fps);
	write(fd, header, n);

	w = width;
	h = height;
	targetFps = fps;
	timeStep = 1000000 / fps;
}

static void writeFrame(int fd, uint32_t width, uint32_t height)
{
	static const char header[] = "FRAME\n";
	write(fd, header, sizeof(header) - 1);

	uint8_t *data = (uint8_t *) buffer;

	for (uint32_t y = height - 1; y < height; --y) {
		write(fd, data + y * width, width);
	}

	data += width * height;

	for (int i = 0; i < 2; ++i) {
		for (uint32_t y = (height / 2) - 1; y < (height / 2); --y) {
			write(fd, data + y * (width / 2), (width / 2));
		}
		data += width * height / 4;
	}
}

static uint64_t diff(uint64_t t1, uint64_t t2)
{
	return t1 < t2 ? t2 - t1 : t1 - t2;
}

void y4mWriteData(int fd, struct yukonPacket *packet, void *data, unsigned int size)
{
	if (pkt.time == 0) {
		pkt = *packet;
		memcpy(buffer, data, size);
		timeNext = pkt.time;
	} else if (diff(pkt.time, timeNext) > diff(packet->time, timeNext)) {
		pkt = *packet;
		memcpy(buffer, data, size);
	} else {
		writeFrame(fd, w, h);
		timeNext += timeStep;
		pkt = *packet;
		memcpy(buffer, data, size);
	}
}
