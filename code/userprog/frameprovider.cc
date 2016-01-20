#ifdef CHANGED

#include "frameprovider.h"


FrameProvider::FrameProvider(int NumPhysPages)
{
	framesMap = new BitMap(NumPhysPages);
	// RandomInit(NumPhysPages);	// Random page allocation
}

FrameProvider::~FrameProvider()
{
	delete framesMap;
}

int
FrameProvider::GetEmptyFrame()
{
	int frameNb = framesMap->Find();	

	// int frameNb = Random()*100;
	
	// while(framesMap->Test(frameNb))	// while frameNbth bit is set, find next one
	// {
	// 	frameNb = Random()*;
	// }
	// TODO	
	if(frameNb == -1)
	{
		// Raise Excepion - OutOfMemory Error - add new system call to handle this excepiton 
	}

	// framesMap->Mark(frameNb);

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