#ifdef CHANGED

#include "frameprovider.h"


FrameProvider::FrameProvider(int NumPhysPages, int PageSize, char* mainMemory)
{
	framesMap = new BitMap(NumPhysPages);
	physPageSize = PageSize;
	memory = mainMemory;
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
	bzero (memory + frameNb * physPageSize, physPageSize);
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