#ifdef CHANGED

#include "frameprovider.h"


FrameProvider::FrameProvider(int NumPhysPages)
{
	framesMap = new BitMap(NumPhysPages);
}

FrameProvider::~FrameProvider()
{
	delete framesMap;
}

int
FrameProvider::GetEmptyFrame()
{
	int frameNb = framesMap->Find();	

	if(frameNb == -1)
	{
		/* Raise Excepion - OutOfMemory Error - add new system call to handle this excepiton */
	}

	return frameNb;
}

void
FrameProvider::ReleaseFrame(int frameNb)
{
	framesMap->Clear(frameNb);
}

int
FrameProvider::NumAvailFrames()
{
	return framesMap->NumClear();
}

#endif /* CHANGED */