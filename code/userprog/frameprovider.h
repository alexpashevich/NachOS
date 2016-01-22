#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include "bitmap.h"

class FrameProvider
{
	public:
		FrameProvider(int NumPhysPages, int PageSize, char* mainMemory);
		~FrameProvider();

		int GetEmptyFrame(void);
		void ReleaseFrame(int frameNb);
		int NumAvailFrames(void);

	private:
		int physPageSize;
		BitMap* framesMap;
		char* memory;
};

#endif