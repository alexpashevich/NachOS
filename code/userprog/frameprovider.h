#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include "bitmap.h"

class FrameProvider
{
	public:
		FrameProvider(int NumPhysPages);
		~FrameProvider();

		int GetEmptyFrame(void);
		void ReleaseFrame(int frameNb);
		int NumAvailFrames(void);

	private:
		
		BitMap* framesMap;
};

#endif